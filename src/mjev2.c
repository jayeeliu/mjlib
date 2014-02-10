#include "mjev2.h"
#include "mjlog.h"
#include "mjcomm.h"
#include <stdlib.h>
#include <sys/epoll.h>

/*
===============================================================================
mjev2_insert_timer_event
===============================================================================
*/
static void mjev2_insert_timer(mjev2 ev2, mjtevt tevt) {
  struct rb_node **new = &(ev2->_troot.rb_node), *parent = NULL;
  while (*new) {
    mjtevt curr = rb_entry(*new, struct mjtevt, _tnode);
    parent = *new;
    if (tevt->_expire < curr->_expire) {
      new = &((*new)->rb_left);
    } else {
      new = &((*new)->rb_right);
    }
  }
  rb_link_node(&tevt->_tnode, parent, new);
  rb_insert_color(&tevt->_tnode, &ev2->_troot);
}

/*
===============================================================================
mjev2_getmin_timer
===============================================================================
*/
static mjtevt mjev2_getmin_timer(mjev2 ev2) {
  struct rb_node *node = rb_first(&ev2->_troot);
  if (!node) return NULL;
  return rb_entry(node, struct mjtevt, _tnode);
}

/*
===============================================================================
mjev2_add_timer
===============================================================================
*/
mjtevt mjev2_add_timer(mjev2 ev2, long long ms, mjtevtProc Handle, 
    void* data) {
  if (!ev2 || ms < 0) return NULL;
  mjtevt tevt = calloc(1, sizeof(struct mjtevt));
  if (!tevt) return NULL; 
  tevt->_expire = get_current_time() + ms;
  tevt->_Handle = Handle;
  tevt->_data   = data;
  rb_init_node(&tevt->_tnode);
  mjev2_insert_timer(ev2, tevt);
  return tevt;
}

bool mjev2_mod_timer(mjev2 ev2, mjtevt tevt, long long ms, mjtevtProc Handle,
    void* data) {
  if (!ev2 || !tevt || ms < 0) return false;
  tevt->_expire = get_current_time() + ms;
  tevt->_Handle = Handle;
  tevt->_data   = data;
  if (!RB_EMPTY_NODE(&tevt->_tnode)) {
    rb_erase(&tevt->_tnode, &ev2->_troot);
  }
  rb_init_node(&tevt->_tnode);
  mjev2_insert_timer(ev2, tevt);
  return true;
}

/*
===============================================================================
mjev2_del_timer
===============================================================================
*/
bool mjev2_del_timer(mjev2 ev2, mjtevt tevt) {
  if (!ev2 || !tevt) return false;
  if (!RB_EMPTY_NODE(&tevt->_tnode)) {
    rb_erase(&tevt->_tnode, &ev2->_troot);
  }
  free(tevt);
  return true;
}

/*
===============================================================================
mjev2_check
===============================================================================
*/
bool mjev2_check(mjev2 ev2) {
  if (!ev2) return false;
  mjtevt tevt = mjev2_getmin_timer(ev2);
  if (!tevt) return false;
  long long curr_time = get_current_time();
  long long timeWait = -1;
  timeWait = (tevt->_expire < curr_time) ? 0 : tevt->_expire - curr_time;
  if (timeWait == -1 || timeWait > 500) timeWait = 500;
  struct epoll_event epEvents[MJEV2_MAXFD];
  epoll_wait(ev2->_epfd, epEvents, MJEV2_MAXFD, timeWait);
  return true;
}

/*
===============================================================================
mjev2_run
===============================================================================
*/
bool mjev2_run(mjev2 ev2) {
  if (!ev2) return false;
  // run timer handle
  long long curr_time = get_current_time();
  mjtevt tevt = mjev2_getmin_timer(ev2);
  while (tevt && tevt->_expire <= curr_time) {
    rb_erase(&tevt->_tnode, &ev2->_troot);
    rb_init_node(&tevt->_tnode);
    if (tevt->_Handle) tevt->_Handle(ev2, tevt);
    tevt = mjev2_getmin_timer(ev2);
  }
  // run handle
  mjevt entry, tmp;
  list_for_each_entry_safe(entry, tmp, &ev2->_rhead, _rnode) {
    list_del(&entry->_rnode);
  }
  return true;
}

/*
===============================================================================
mjev2_new
===============================================================================
*/
mjev2 mjev2_new() {
  mjev2 ev2 = calloc(1, sizeof(struct mjev2));
  if (!ev2) {
    MJLOG_ERR("mjev2 alloc error");
    return NULL;
  }
  ev2->_epfd = epoll_create(1024);
  if (ev2->_epfd == -1) {
    MJLOG_ERR("epoll_create error");
    free(ev2);
    return NULL;
  }

  INIT_LIST_HEAD(&ev2->_rhead);
  ev2->_troot = RB_ROOT;
  return ev2;
}

/*
===============================================================================
mjev2_delete
===============================================================================
*/
bool mjev2_delete(mjev2 ev2) {
  if (!ev2) return false;
  close(ev2->_epfd);
  free(ev2);
  return true;
}

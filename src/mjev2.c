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
static bool mjev2_insert_timer_event(mjev2 ev2, mjevt evt) {
  struct rb_node **new = &(ev2->_troot.rb_node), *parent = NULL;
  while (*new) {
    mjevt curr = rb_entry(*new, struct mjevt, _tnode);
    parent = *new;
    if (evt->_expire < curr->_expire) {
      new = &((*new)->rb_left);
    } else {
      new = &((*new)->rb_right);
    }
  }
  rb_link_node(&evt->_tnode, parent, new);
  rb_insert_color(&evt->_tnode, &ev2->_troot);
  return true;
}

/*
===============================================================================
mjev2_getmin_timer_event
===============================================================================
*/
static mjevt mjev2_getmin_timer_event(mjev2 ev2) {
  struct rb_node *node = rb_first(&ev2->_troot);
  if (!node) return NULL;
  return rb_entry(node, struct mjevt, _tnode);
}

/*
===============================================================================
mjev2_add_timer
===============================================================================
*/
mjevt mjev2_add_timer(mjev2 ev2, long long ms, mjevtProc Handle, void* data) {
  if (!ev2) return NULL;
  mjevt evt = calloc(1, sizeof(struct mjevt));
  evt->_fd            = -1;  
  evt->_TimeoutHandle = Handle; 
  evt->_data          = data;
  evt->_expire        = ms;
  INIT_LIST_HEAD(&evt->_pnode);
  rb_init_node(&evt->_tnode);
  mjev2_insert_timer_event(ev2, evt);
  return evt;
}

/*
===============================================================================
mjev2_del_timer
===============================================================================
*/
bool mjev2_del_timer(mjev2 ev2, mjevt evt) {
  if (!ev2 || !evt || evt->_fd != -1) return false;
  rb_erase(&evt->_tnode, &ev2->_troot);
  return true;
}

/*
===============================================================================
mjev2_check
===============================================================================
*/
bool mjev2_check(mjev2 ev2) {
  if (!ev2) return false;
  long long curr_time = get_current_time();
  mjevt evt = mjev2_getmin_timer_event(ev2);
  while (evt && evt->_expire <= curr_time) {
    mjev2_del_timer(ev2, evt);
    evt->_timeout = true;
    list_add_tail(&evt->_pnode, &ev2->_phead);
    evt = mjev2_getmin_timer_event(ev2);
  }
  return true;
}

/*
===============================================================================
mjev2_run
===============================================================================
*/
bool mjev2_run(mjev2 ev2) {
  if (!ev2) return false;
  mjevt entry, tmp;
  list_for_each_entry_safe(entry, tmp, &ev2->_phead, _pnode) {
    list_del(&entry->_pnode);
    if (entry->_timeout && entry->_TimeoutHandle) {
      entry->_TimeoutHandle(entry);
    }
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

  INIT_LIST_HEAD(&ev2->_phead);
  INIT_LIST_HEAD(&ev2->_fhead);
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

#include "mjev2.h"
#include "mjlog.h"
#include "mjcomm.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>

/*
===============================================================================
mjev2_add_timerlist
===============================================================================
*/
static void mjev2_add_timerlist(mjev2 ev2, mjtevt tevt) {
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
mjev2_del_timerlist
===============================================================================
*/
static inline void mjev2_del_timerlist(mjev2 ev2, mjtevt tevt) {
  if (!RB_EMPTY_NODE(&tevt->_tnode)) {
    rb_erase(&tevt->_tnode, &ev2->_troot);
  }
  rb_init_node(&tevt->_tnode);
}

/*
===============================================================================
mjev2_get_min_timer
===============================================================================
*/
static inline mjtevt mjev2_get_min_timer(mjev2 ev2) {
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
  mjev2_del_timerlist(ev2, tevt);
  mjev2_add_timerlist(ev2, tevt);
  return tevt;
}

/*
===============================================================================
mjev2_mod_timer
===============================================================================
*/
bool mjev2_mod_timer(mjev2 ev2, mjtevt tevt, long long ms, mjtevtProc Handle,
    void* data) {
  if (!ev2 || !tevt || ms < 0) return false;
  tevt->_expire = get_current_time() + ms;
  tevt->_Handle = Handle;
  tevt->_data   = data;
  mjev2_del_timerlist(ev2, tevt);
  mjev2_add_timerlist(ev2, tevt);
  return true;
}

/*
===============================================================================
mjev2_del_timer
===============================================================================
*/
bool mjev2_del_timer(mjev2 ev2, mjtevt tevt) {
  if (!ev2 || !tevt) return false;
  mjev2_del_timerlist(ev2, tevt);
  free(tevt);
  return true;
}

/*
===============================================================================
mjev2_event_timeout
===============================================================================
*/
static void mjev2_event_timeout(mjev2 ev2, mjtevt tevt) {
  // set timeout flag
  mjevt evt = tevt->_data;
  if (tevt == &evt->_rtevt) {
    evt->_readTimeout = true;
  } else {
    evt->_writeTimeout = true;
  }
  // insert to running queue
  if (list_empty(&evt->_rnode)) list_add_tail(&evt->_rnode, &ev2->_rhead);
}

/*
===============================================================================
mjev2_init_epoll_event
===============================================================================
*/
static inline void mjev2_init_epoll_event(struct epoll_event *ee, int fd,
    int mask) {
  ee->data.u64 = 0;
  ee->data.fd  = fd;
  ee->events   = 0;
  if (mask & MJEV2_READ) ee->events |= EPOLLIN;
  if (mask & MJEV2_WRITE) ee->events |= EPOLLOUT;
}

/*
===============================================================================
mjev2_add_event
===============================================================================
*/
bool mjev2_add_event(mjev2 ev2, int fd, int mask, mjevtProc Handle, 
    void* data, long long ms) {
  if (!ev2 || fd < 0 || fd > MJEV2_MAXFD || !Handle) return false;
  if ((mask & (MJEV2_READ | MJEV2_WRITE)) == 0) return false;

  mjevt evt = &ev2->_events[fd];
  int newmask = evt->_mask | mask;
  if (evt->_mask != newmask) {
    struct epoll_event ee;
    mjev2_init_epoll_event(&ee, fd, newmask);
    // add event
    int op = (evt->_mask == MJEV2_NONE) ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    if (epoll_ctl(ev2->_epfd, op, fd, &ee) == -1) {
      MJLOG_ERR("epoll_ctl error");
      return false;
    }
    evt->_mask = newmask;
  }
  // we must change Handle and data
  if (mask & MJEV2_READ) evt->_ReadHandle = Handle;
  if (mask & MJEV2_WRITE) evt->_WriteHandle = Handle;
  evt->_data = data;
  // set event timeout
  if (ms > 0) {
    if (mask & MJEV2_READ) {
      mjev2_mod_timer(ev2, &evt->_rtevt, ms, mjev2_event_timeout, evt);
    }
    if (mask & MJEV2_WRITE) { 
      mjev2_mod_timer(ev2, &evt->_wtevt, ms, mjev2_event_timeout, evt);
    }
  }
  return true;
}


/*
===============================================================================
mjev2_del_event
===============================================================================
*/
bool mjev2_del_event(mjev2 ev2, int fd, int mask) {
  if (!ev2 || fd < 0 || fd > MJEV2_MAXFD) return false;
  if ((mask & (MJEV2_READ | MJEV2_WRITE)) == 0) return false;
  mjevt evt = &ev2->_events[fd];
  int newmask = evt->_mask & (~mask);
  if (evt->_mask != newmask) {
    struct epoll_event ee;
    mjev2_init_epoll_event(&ee, fd, newmask);
    // modify or del event
    int op = (newmask == MJEV2_NONE) ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
    if (epoll_ctl(ev2->_epfd, op, fd, &ee) == -1) {
      MJLOG_ERR("epoll_ctl error");
      return false;
    }
    evt->_mask = newmask;
    // del timeout event
    if (mask & MJEV2_READ) mjev2_del_timerlist(ev2, &evt->_rtevt);
    if (mask & MJEV2_WRITE) mjev2_del_timerlist(ev2, &evt->_wtevt);
  }
  return true;
}


/*
===============================================================================
mjev2_check_event
===============================================================================
*/
bool mjev2_check_event(mjev2 ev2) {
  if (!ev2) return false;
  struct epoll_event epEvents[MJEV2_MAXFD];
  int numevents = epoll_wait(ev2->_epfd, epEvents, MJEV2_MAXFD, 300);
  if (numevents < 0) {
    if (errno == EINTR) return false;
    MJLOG_ERR("epoll_wait error. errno: %d, msg: %s", errno, strerror(errno));
    return false;
  }
  // check and set status
  for (int i = 0; i < numevents; i++) {
    struct epoll_event* e = &epEvents[i];
    int fd = e->data.fd;
    mjevt evt = &ev2->_events[fd];
    if (e->events & EPOLLIN) evt->_readReady = true;
    if (e->events & EPOLLOUT) evt->_writeReady = true;
    if (e->events & EPOLLERR) evt->_error = true;
    if (list_empty(&evt->_rnode)) list_add_tail(&evt->_rnode, &ev2->_rhead);
  }
  return true;
}

/*
===============================================================================
mjev2_run_timer
===============================================================================
*/
bool mjev2_run_timer(mjev2 ev2) {
  if (!ev2) return false;
  mjtevt tevt = mjev2_get_min_timer(ev2);
  if (!tevt) return true;

  long long curr_time = get_current_time();
  while (tevt && tevt->_expire <= curr_time) {
    mjev2_del_timerlist(ev2, tevt);
    if (tevt->_Handle) tevt->_Handle(ev2, tevt);
    tevt = mjev2_get_min_timer(ev2);
  }
  return true;
}

/*
===============================================================================
mjev2_run_event
===============================================================================
*/
bool mjev2_run_event(mjev2 ev2) {
  if (!ev2) return false;
  mjevt evt, tmp, saved;
  list_for_each_entry_safe(evt, tmp, &ev2->_rhead, _rnode) {
    list_del_init(&evt->_rnode);

    saved = evt;
    if (evt && (evt->_readTimeout || evt->_readReady) && evt->_ReadHandle) {
      evt = evt->_ReadHandle(ev2, evt);
    }
    if (evt && (evt->_writeTimeout || evt->_writeReady) && evt->_WriteHandle) {
      evt = evt->_WriteHandle(ev2, evt);
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

  INIT_LIST_HEAD(&ev2->_rhead);
  ev2->_troot = RB_ROOT;

  for (int i = 0; i < MJEV2_MAXFD; i++) {
    mjevt evt = &(ev2->_events[i]);
    INIT_LIST_HEAD(&evt->_rnode);
    rb_init_node(&evt->_rtevt._tnode);
    rb_init_node(&evt->_wtevt._tnode);
  }
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

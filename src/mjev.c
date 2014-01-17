#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include "mjev.h"
#include "mjcomm.h"
#include "mjlog.h"

/*
===============================================================================
mjev_Add 
    Add FileEvent to Loop, fd--socket handler
    mask -- MJEV_READABLE(for read) or MJEV_WRITEABLE(for write)
    proc -- event handle function, data -- args for proc
    output -1-- add event failed,  0--add event success
===============================================================================
*/
bool mjev_add_fevent(mjev ev, int fd, int mask, mjProc RT, void* arg) {
  // sanity check
  if (!ev || !RT) return false;
  if (fd >= MJEV_MAXFD || fd < 0) {
    MJLOG_ERR("fd is invalid: %d", fd);
    return false;
  }
  if ((mask & MJEV_READABLE) == 0 && (mask & MJEV_WRITEABLE) == 0) {
    MJLOG_ERR("only support READ and WRITE: %d", mask);
    return false; 
  }
  // get mjfevent correspond to fd
  mjfevent fdev = &(ev->_fevent_list[fd]);     
  int newmask = fdev->_mask | mask;            // get new mask 
  if (fdev->_mask != newmask) {
    // we should change epoll, set epoll event
    struct epoll_event ee;
    ee.data.u64 = 0;
    ee.data.fd  = fd;
    ee.events   = 0;
    if (newmask & MJEV_READABLE) ee.events |= EPOLLIN;
    if (newmask & MJEV_WRITEABLE) ee.events |= EPOLLOUT;
    // add event
    int op = (fdev->_mask == MJEV_NONE) ? EPOLL_CTL_ADD : EPOLL_CTL_MOD; 
    if (epoll_ctl(ev->_epfd, op, fd, &ee) == -1) {
      MJLOG_ERR("epoll_ctl error");
      return false;
    }
    fdev->_mask = newmask;
  }
  // we must change Proc and Proc data
  if (mask & MJEV_READABLE) fdev->_RRT = RT;
  if (mask & MJEV_WRITEABLE) fdev->_WRT = RT;
  fdev->_arg = arg;
  return true;
}

/*
===============================================================================
mjev_Del
    delete event from mjev
===============================================================================
*/
bool mjev_del_fevent(mjev ev, int fd, int mask) {
  // sanity check
  if (!ev) return false;
  if (fd >= MJEV_MAXFD || fd < 0) {
    MJLOG_ERR("fd is invalid");
    return false;
  }
  if ((mask & MJEV_READABLE) == 0 && (mask & MJEV_WRITEABLE) == 0) {
    MJLOG_ERR("only support READ and WRITE: %d", mask);
    return false; 
  }
  // set mjev status
  mjfevent fdev = &ev->_fevent_list[fd];
  int newmask = fdev->_mask & (~mask);         // get newmask
  if (fdev->_mask != newmask) { 
    // we should change epoll, set epoll event
    struct epoll_event ee;
    ee.data.u64 = 0;
    ee.data.fd  = fd;
    ee.events   = 0;
    if (newmask & MJEV_READABLE) ee.events |= EPOLLIN;
    if (newmask & MJEV_WRITEABLE) ee.events |= EPOLLOUT;
    // modify or delete event
    int op = (newmask == MJEV_NONE) ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
    if (epoll_ctl(ev->_epfd, op, fd, &ee) == -1) {
      MJLOG_ERR("epoll_ctl error");
      return false;
    }
    fdev->_mask = newmask;
  }
  return true;
}

/*
===============================================================================
mjev_add_timer
    add timer event to mjev
===============================================================================
*/
mjtevent mjev_add_timer(mjev ev, long long ms, mjProc TRT, void* arg) {
  if (!ev || !TRT) return NULL; 
  // create mjTimerEvent struct
  mjtevent te = (mjtevent) calloc(1, sizeof(struct mjtevent));
  if (!te) {
    MJLOG_ERR("mjtevent alloc error");
    return NULL; 
  }
  // init timerevent struct
  te->_valid  = true;                 
  te->_time   = ms + get_current_time();
  te->_TRT    = TRT;
  te->_arg    = arg;
  // insert into queue
  if (mjpq_insert(ev->_tevent_queue, te->_time, te) < 0) {
    MJLOG_ERR("mjpq_insert error");
    free(te);
    return NULL;
  }
  return te;
}

/*
===============================================================================
mjev_del_timer
    invalid mjtevent
===============================================================================
*/
bool mjev_del_timer(mjev ev, mjtevent te) {
  if (!ev || !te) return false;
  te->_valid = false;
  return true;
}

/*
===============================================================================
mjev_add_pending
    add pending proc to ev
===============================================================================
*/
bool mjev_add_pending(mjev ev, mjProc PRT, void* arg) {
  if (!ev || !PRT || !arg) return false;
  // alloc and set new pending
  mjpending newPending = (mjpending) calloc(1, sizeof(struct mjpending));
  if (!newPending) {
    MJLOG_ERR("pending struct alloc error");
    return false;
  }
  newPending->_PRT  = PRT;
  newPending->_arg  = arg;
  INIT_LIST_HEAD(&newPending->_pnode);
  list_add_tail(&newPending->_pnode, &ev->_phead); 
  return true;
}

/*
===============================================================================
mjev_del_pending
    del pending proc according to data.
    it is useful. when error happends
    data is freed, before pending proc 
    called.
===============================================================================
*/
bool mjev_del_pending(mjev ev, void* arg) {
  if (list_empty(&ev->_phead)) return false;
  mjpending entry, tmp;
  list_for_each_entry_safe(entry, tmp, &ev->_phead, _pnode) {
    if (entry->_arg == arg) {
      list_del(&entry->_pnode);
      free(entry);
    }
  }
  return true; 
}

/*
===============================================================================
GetFirstTimer
    get first timer from timer queue
===============================================================================
*/
static inline long long get_first_timer(mjev ev) {
  return mjpq_get_minkey(ev->_tevent_queue);
}

/*
===============================================================================
GetFirstTimerEvent
    get first time event from queue
===============================================================================
*/
static inline mjtevent get_first_timerevent(mjev ev) {
  mjtevent te = mjpq_get_minvalue(ev->_tevent_queue);
  if (!te) return NULL;
  // delete timer event from queue
  mjpq_delmin(ev->_tevent_queue);
  return te;
}

/*
===============================================================================
mjev_Run
    run event loop
===============================================================================
*/
void mjev_run(mjev ev) {
  if (!ev) return;
  // epoll_wait timeout, -1 wait forver 
  long long timeWait = -1;
  // get first timer event from queue
  long long currTime;  
  long long first = get_first_timer(ev); 
  if (first != -1) {          
    // adjust wait time, 0 return immediate
    currTime = get_current_time();
    timeWait = (first <= currTime) ? 0 : first - currTime;
  }
  // have pending proc run, no wait
  if (!list_empty(&ev->_phead)) timeWait = 0;
  // loop at least 500ms
  if (timeWait == -1 || timeWait > 500) timeWait = 500;
  // wait for event, or timeout
  struct epoll_event epEvents[MJEV_MAXFD];
  int numevents = epoll_wait(ev->_epfd, epEvents, MJEV_MAXFD, timeWait);
  if (numevents < 0) {
    if (errno == EINTR) return;
    MJLOG_ERR("epoll_wait error. errno: %d, msg: %s", errno, strerror(errno));
    return; // some error
  }
  // stage1: get timer event, run it
  first = get_first_timer(ev);                       
  currTime = get_current_time();        
  if (first != -1 && first <= currTime) {
    mjtevent te = get_first_timerevent(ev);
    if (te && te->_valid && te->_TRT) te->_TRT(te->_arg);
    free(te);                                  
  }
  if (!numevents && list_empty(&ev->_phead)) return;  // no events, break
  // stage2: get events from list, runner file event
  for (int i = 0; i < numevents; i++) {
    // get file event fd, and mjfevent struct
    struct epoll_event* e = &epEvents[i];
    int fd = e->data.fd;
    mjfevent fdev = &ev->_fevent_list[fd];
    // check file op mask
    int mask = 0;
    if (e->events & EPOLLIN) mask |= MJEV_READABLE;
    if (e->events & EPOLLOUT) mask |= MJEV_WRITEABLE;
    // check epoll error
    if (mask == 0 && (e->events & EPOLLERR)) mask |= MJEV_READABLE | MJEV_WRITEABLE;
    // run proc according to mask
    int rFired = 0;
    if (fdev->_mask & mask & MJEV_READABLE) {
      // we have run fileproc
      rFired = 1;
      fdev->_RRT(fdev->_arg);
    }
    if (fdev->_mask & mask & MJEV_WRITEABLE) {
      if (!rFired || fdev->_WRT != fdev->_RRT) fdev->_WRT(fdev->_arg);
    }
  }
  // stage 3: check and run pending proc
  if (list_empty(&ev->_phead)) return;
  struct list_head savedPendingHead;
  INIT_LIST_HEAD(&savedPendingHead);
  list_splice_init(&ev->_phead, &savedPendingHead);
  // run pending proc
  mjpending entry, tmp;
  list_for_each_entry_safe(entry, tmp, &savedPendingHead, _pnode) {
    list_del(&entry->_pnode);
    entry->_PRT(entry->_arg);
    free(entry);
  }
}

/*
===============================================================================
mjev_New
    create mjev struct
    return NULL --- failed
===============================================================================
*/
mjev mjev_new() {
  // create mjev struct 
  mjev ev = (mjev) calloc(1, sizeof(struct mjev));
  if (!ev) {
    MJLOG_ERR("mjev malloc error");
    return NULL;
  }
  // create new epoll
  ev->_epfd = epoll_create(1024);
  if (ev->_epfd == -1) {
    MJLOG_ERR("epoll_create error");
    free(ev);
    return NULL;
  }
  // create timer event queue
  ev->_tevent_queue = mjpq_new();
  if (!ev->_tevent_queue) {
    MJLOG_ERR("mjpq alloc error");
    close(ev->_epfd);
    free(ev);
    return NULL;
  }
  // init pending list
  INIT_LIST_HEAD(&ev->_phead);
  return ev;
}

/*
===============================================================================
mjev_release_tevent
  mjev release timer event
===============================================================================
*/
static void mjev_release_tevent(mjev ev) {
  mjtevent te = get_first_timerevent(ev);
  while (te) {
    free(te);
    te = get_first_timerevent(ev);
  }
  mjpq_delete(ev->_tevent_queue);
}

/*
===============================================================================
mjev_release_pending
    release Pending struct
===============================================================================
*/
static void mjev_release_pending(mjev ev) {
  if (list_empty(&ev->_phead)) return;
  // release pending struct 
  mjpending entry, tmp;
  list_for_each_entry_safe(entry, tmp, &ev->_phead, _pnode) {
    list_del(&entry->_pnode);
    free(entry);
  }
}

/*
===============================================================================
mjev_Delete
    Delete mjev object. 
    No Return.
===============================================================================
*/
bool mjev_delete(mjev ev) {
  // sanity check
  if (!ev) return false;
  close(ev->_epfd);
  // release tevent and pending struct
  mjev_release_tevent(ev);
  mjev_release_pending(ev);
  free(ev);
  return true;
}

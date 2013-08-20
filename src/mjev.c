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
bool mjev_add_fevent(mjev ev, int fd, int mask, mjProc Proc, void* arg) {
  // sanity check
  if (!ev || !Proc) {
    MJLOG_ERR("mjev or Proc is null");
    return false;
  }
  // fd must not large than MJEV_MAXFD
  if (fd >= MJEV_MAXFD || fd < 0) {
    MJLOG_ERR("fd is invalid: %d", fd);
    return false;
  }
  // mask check
  if ((mask & MJEV_READABLE) == 0 && (mask & MJEV_WRITEABLE) == 0) {
      MJLOG_ERR("only support READ and WRITE: %d", mask);
      return false; 
  }
  // get mjfevent correspond to fd
  mjfevent fdev = &(ev->_file_event_list[fd]);     
  int newmask = fdev->_mask | mask;            // get new mask 
  // we should change epoll
  if (fdev->_mask != newmask) {
    // set epoll event
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
    // set newmask
    fdev->_mask = newmask;
  }
  // we must change Proc and Proc data
  if (mask & MJEV_READABLE) fdev->_ReadProc = Proc;
  if (mask & MJEV_WRITEABLE) fdev->_WriteProc = Proc;
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
  if (!ev) {
    MJLOG_ERR("ev is null");
    return false;
  }
  // input parameter check 
  if (fd >= MJEV_MAXFD || fd < 0) {
    MJLOG_ERR("fd is invalid");
    return false;
  }
  if ((mask & MJEV_READABLE) == 0 && (mask & MJEV_WRITEABLE) == 0) {
    MJLOG_ERR("only support READ and WRITE: %d", mask);
    return false; 
  }
  // set mjev status
  mjfevent fdev = &ev->_file_event_list[fd];
  int newmask = fdev->_mask & (~mask);         // get newmask
  // we should change epoll
  if (fdev->_mask != newmask) { 
    // set epoll event
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
    // set newmask
    fdev->_mask = newmask;
  }
  return true;
}

/*
===============================================================================
mjev_AddTimer
    add timer event to mjev
===============================================================================
*/
mjtevent mjev_add_timer(mjev ev, long long ms, mjProc TimerProc, void* arg) {
  if (!ev || !TimerProc) {
    MJLOG_ERR("ev or Proc is null");
    return NULL;
  }
  // create mjTimerEvent struct
  mjtevent te = (mjtevent) calloc(1, sizeof(struct mjtevent));
  if (!te) {
    MJLOG_ERR("mjtevent alloc error");
    return NULL; 
  }
  // init timerevent struct
  te->_valid      = true;                 
  te->_time       = ms + GetCurrentTime();
  te->_TimerProc  = TimerProc;
  te->_arg        = arg;
  // insert into queue
  if (mjpq_insert(ev->_timer_event_queue, te->_time, te) < 0) {
    MJLOG_ERR("mjpq_insert error");
    free(te);
    return NULL;
  }
  return te;
}

/*
===============================================================================
mjev_DelTimer
    invalid mjtevent
===============================================================================
*/
bool mjev_del_timer(mjev ev, mjtevent te) {
  if (!ev || !te) {
    MJLOG_ERR("ev or te is null");
    return false;
  }
  te->_valid = false;
  return true;
}

/*
===============================================================================
mjev_Pending
    add pending proc to ev
===============================================================================
*/
bool mjev_add_pending(mjev ev, mjProc PendingProc, void* arg) {
  if (!ev || !PendingProc) {
    MJLOG_ERR("ev or Proc is null");
    return false;
  }
  // alloc and set new pending
  mjpending newPending = (mjpending) calloc(1, sizeof(struct mjpending));
  if (!newPending) {
    MJLOG_ERR("pending struct alloc error");
    return false;
  }
  newPending->_PendingProc = PendingProc;
  newPending->_arg  = arg;
  INIT_LIST_HEAD(&newPending->_pending_node);
  list_add_tail(&newPending->_pending_node, &ev->_pending_head); 
  return true;
}

/*
===============================================================================
mjev_DelPending
    del pending proc according to data.
    it is useful. when error happends
    data is freed, before pending proc 
    called.
===============================================================================
*/
bool mjev_del_pending(mjev ev, void* arg) {
  if (list_empty(&ev->_pending_head)) return false;
  mjpending entry, tmp;
  list_for_each_entry_safe(entry, tmp, &ev->_pending_head, _pending_node) {
    if (entry->_arg == arg) {
      list_del(&entry->_pending_node);
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
static long long get_first_timer(mjev ev) {
  return mjpq_get_minkey(ev->_timer_event_queue);
}

/*
===============================================================================
GetFirstTimerEvent
    get first time event from queue
===============================================================================
*/
static mjtevent get_first_timerevent(mjev ev) {
  mjtevent te = mjpq_get_minvalue(ev->_timer_event_queue);
  if (!te) return NULL;
  // delete timer event from queue
  mjpq_delmin(ev->_timer_event_queue);
  return te;
}

/*
===============================================================================
mjev_Run
    run event loop
===============================================================================
*/
void mjev_run(mjev ev) {
  // sanity check
  if (!ev) {
    MJLOG_ERR("mjev is null");
    return;
  }
  // epoll_wait timeout, -1 wait forver 
  long long timeWait = -1;
  // current time clock
  long long currTime;  
  // get first timer event from queue
  long long first = get_first_timer(ev); 
  // we got a timer event
  if (first != -1) {          
    currTime = GetCurrentTime();
    // adjust wait time, 0 return immediate
    timeWait = (first <= currTime) ? 0 : first - currTime;
  }
  // have pending proc run, no wait
  if (!list_empty(&ev->_pending_head)) timeWait = 0;
  // wait for event
  struct epoll_event epEvents[MJEV_MAXFD];
  int numevents = epoll_wait(ev->_epfd, epEvents, MJEV_MAXFD, timeWait);
  if (numevents < 0) {
    if (errno == EINTR) return;
    MJLOG_ERR("epoll_wait error. errno: %d, msg: %s", errno, strerror(errno));
    return; // some error
  }
  // get timer event, run it
  first = get_first_timer(ev);                       
  currTime = GetCurrentTime();        
  if (first != -1 && first <= currTime) {           // run timer event
    mjtevent te = get_first_timerevent(ev);        // get timer event from queue
    if (te && te->_valid && te->_TimerProc) te->_TimerProc(te->_arg);   // call timer proc
    free(te);                                       // free timer event, alloc in addtimer
  }
  if (!numevents && list_empty(&ev->_pending_head)) return;  // no events, break
  // get events from list
	for (int i = 0; i < numevents; i++) {
    // get file event fd, and mjfevent struct
	  struct epoll_event* e = &epEvents[i];
	  int fd = e->data.fd;
	  mjfevent fdev = &ev->_file_event_list[fd];
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
		  fdev->_ReadProc(fdev->_arg);
	  }
	  if (fdev->_mask & mask & MJEV_WRITEABLE) {
		  if (!rFired || fdev->_WriteProc != fdev->_ReadProc) {
			  fdev->_WriteProc(fdev->_arg);
			}
		}
	}
  // check and run pending proc
  if (list_empty(&ev->_pending_head)) return;
  struct list_head savedPendingHead;
  INIT_LIST_HEAD(&savedPendingHead);
  list_splice_init(&ev->_pending_head, &savedPendingHead);
  // run pending proc
  mjpending entry, tmp;
  list_for_each_entry_safe(entry, tmp, &savedPendingHead, _pending_node) {
    list_del(&entry->_pending_node);
    entry->_PendingProc(entry->_arg);
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
  ev->_timer_event_queue = mjpq_new();
  if (!ev->_timer_event_queue) {
    MJLOG_ERR("mjpq alloc error");
    close(ev->_epfd);
    free(ev);
    return NULL;
  }
  // init pending list
  INIT_LIST_HEAD(&ev->_pending_head);
  return ev;
}

/*
===============================================================================
mjev_ReleasePending
    release Pending struct
===============================================================================
*/
static void mjev_release_pending(mjev ev) {
  if (list_empty(&ev->_pending_head)) return;
  // release pending struct 
  mjpending entry, tmp;
  list_for_each_entry_safe(entry, tmp, &ev->_pending_head, _pending_node) {
    list_del(&entry->_pending_node);
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
  if (!ev) {
    MJLOG_ERR("mjev is null");
    return false;
  }
  close(ev->_epfd);
  mjpq_delete(ev->_timer_event_queue);
  // release pending struct
  mjev_release_pending(ev);
  free(ev);
  return true;
}

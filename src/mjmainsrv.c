#define _GNU_SOURCE
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include "mjmainsrv.h"
#include "mjconn.h"
#include "mjcomm.h"
#include "mjsock.h"
#include "mjsig.h"
#include "mjlog.h"

struct mjmainsrv_asy_d {
  int     _notify_r;
  int     _notify_w;
  mjev    _ev;
  mjProc  _Routine;
  void*   _arg;
  mjProc  _CallBack;
  void*   _c_arg;
};
typedef struct mjmainsrv_asy_d* mjmainsrv_asy_d;

/*
===============================================================================
mjmainsrv_asy_fin(server routine)
  call when asyncroutine finish
===============================================================================
*/
static void* mjmainsrv_asy_fin(void* data) {
  mjmainsrv_asy_d asy_d = (mjmainsrv_asy_d) data;
  // get and clean
  char buffer[2];
  read(asy_d->_notify_r, buffer, sizeof(buffer));
  // delete event
  mjev_del_fevent(asy_d->_ev, asy_d->_notify_r, MJEV_READABLE);
  close(asy_d->_notify_r);
  close(asy_d->_notify_w);
  free(asy_d);
  // run callback proc
  asy_d->_CallBack(asy_d->_c_arg);
  return NULL;
}

/*
===============================================================================
mjmainsrv_asy_routine(thread routine)
  aync run routine
===============================================================================
*/
static void* mjmainsrv_asy_routine(void* data) {
  // get data from asyncData
  mjthread thread = (mjthread) data;
  mjmainsrv_asy_d asy_d = (mjmainsrv_asy_d) thread->arg;
  // run Routine
  asy_d->_Routine(asy_d->_arg);
  // notify eventloop
  write(asy_d->_notify_w, "OK", 2);
  return NULL;
}

/*
===============================================================================
mjmainsrv_Async
  run workerRoutine in threadpool, when finish call CallBack
===============================================================================
*/
bool mjmainsrv_asy(mjtcpsrv srv, mjProc Routine, void* arg, mjProc CallBack, 
    void* c_arg) {
  // sanity check
  if (!srv) {
    MJLOG_ERR("srv is null");
    return false;
  }
  // alloc mjmainsrv_AsyncData
  mjmainsrv_asy_d asy_d = (mjmainsrv_asy_d) calloc(1,
      sizeof(struct mjmainsrv_asy_d));
  if (!asy_d) {
    MJLOG_ERR("AsycData alloc Error");
    return false;
  }
  asy_d->_ev        = srv->ev;
  asy_d->_Routine   = Routine;
  asy_d->_arg       = arg;
  asy_d->_CallBack  = CallBack;
  asy_d->_c_arg     = c_arg;
  // alloc notify pipe and add callback event to eventloop
  int notify_fd[2];
  if (pipe(notify_fd)) {
    MJLOG_ERR("pipe alloc error: %s", strerror(errno));
    free(asy_d);
    return false;
  }
  asy_d->_notify_r  = notify_fd[0];
  asy_d->_notify_w  = notify_fd[1];
  mjev_add_fevent(srv->ev, notify_fd[0], MJEV_READABLE, mjmainsrv_asy_fin, 
      asy_d);
  // add routine to threadpool
  mjmainsrv mainsrv = (mjmainsrv) mjtcpsrv_get_obj(srv, "mainsrv");
  if (!mjthreadpool_add_routine_plus(mainsrv->_worker_pool, 
      mjmainsrv_asy_routine, asy_d)) {
    MJLOG_ERR("Oops async run Error");
    // del notify event
    mjev_del_fevent(srv->ev, notify_fd[0], MJEV_READABLE);
    close(notify_fd[0]);
    close(notify_fd[1]);
    free(asy_d);
    return false;
  }
  return true;
}

/*
===============================================================================
mjmainsrv_Run
  run main srv. just for accept and dispatch
===============================================================================
*/
bool mjmainsrv_run(mjmainsrv srv) {
  // sanity check
  if (!srv) {
    MJLOG_ERR("srv is null");
    return false;
  }
  // accept and dispatch
  static int dispatch = 0;
  while (!srv->_stop) {
    int cfd = mjsock_accept(srv->_sfd);
    if (cfd < 0) {
      MJLOG_ERR("mjSock_Accept Error continue: %s", strerror(errno));
      continue;
    }
    dispatch = (dispatch + 1) % srv->_srv_num;
    if (write(srv->_srv_n[dispatch], &cfd, sizeof(int)) < 0) {
      MJLOG_ERR("set socket to thread error, close");
      mjsock_close(cfd);
    }
  }
  return true;
}

/*
===============================================================================
mjmainsrv_SetStop
  set main server stop value
===============================================================================
*/
bool mjmainsrv_set_stop(mjmainsrv srv, bool value) {
  if (!srv) return false;
  srv->_stop = value ? true : false;
  return true;
}

/*
===============================================================================
mjmainsrv_srv_run
  wrap function for srvthread_run
===============================================================================
*/
static void* mjmainsrv_srv_run(void* arg) {
  mjthread thread = (mjthread) arg;
  mjtcpsrv srv = mjthread_get_obj(thread, "tcpserver");
  mjtcpsrv_run(srv);
  return NULL;
}

/*
===============================================================================
mjmainsrv_New
  create new mjsrv struct
===============================================================================
*/
mjmainsrv mjmainsrv_new(int sfd, mjProc SrvRoutine, mjProc InitSrv, 
    void* init_arg, int worker_num) {
  // alloc srv struct
  mjmainsrv srv = (mjmainsrv) calloc(1, sizeof(struct mjmainsrv));
  if (!srv) {
    MJLOG_ERR("mjsrv create error");
    return NULL;
  }
  // set listen socket blocking
  mjsock_set_blocking(sfd, 1);
  // update fileds and srv
  srv->_sfd    	= sfd;
  srv->_srv_num	= get_cpu_count();
  if (srv->_srv_num <= 0) {
    MJLOG_ERR("cpu count error");
    free(srv);
    return NULL;
  }
  // create srv
  int fd[2];
  for (int i = 0; i < srv->_srv_num; i++) {
    // set srvNotify
    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, fd)) {
      MJLOG_ERR("socketpair error");
      mjmainsrv_delete(srv);
      return NULL;
    }
    srv->_srv_n[i] = fd[0];
    // create new srv struct and set main srv
    srv->_srv[i] = mjtcpsrv_new(fd[1], SrvRoutine, InitSrv, init_arg, 
        MJTCPSRV_INNER);
    if (!srv->_srv[i]) {
      MJLOG_ERR("mjTcpSrv create error");
      mjmainsrv_delete(srv);
      return NULL;
    }
		mjtcpsrv_set_obj(srv->_srv[i], "mainsrv", srv, NULL);
    // create new thread
    srv->_srv_t[i] = mjthread_new(NULL, NULL);
    if (!srv->_srv_t[i]) {
      MJLOG_ERR("mjThread create error");
      mjmainsrv_delete(srv);
      return NULL;
    }
    mjthread_set_obj(srv->_srv_t[i], "tcpserver", srv->_srv[i], 
        mjtcpsrv_delete);
    mjthread_add_routine(srv->_srv_t[i], mjmainsrv_srv_run, NULL);
  }
  // update threadpool
  srv->_worker_pool = mjthreadpool_new(worker_num, NULL, NULL); 
  if (!srv->_worker_pool) {
    MJLOG_ERR("threadpool create error");
    mjmainsrv_delete(srv);
    return NULL;
  }
  return srv;
}

/*
===============================================================================
mjmainsrv_Delete
  delete mjmainsrv struct
===============================================================================
*/
bool mjmainsrv_delete(mjmainsrv srv) {
  // sanity check
  if (!srv) {
    MJLOG_ERR("srv is null");
    return false;
  }
  // free fd and mjTcpSrv
  for (int i = 0; i < srv->_srv_num; i++) {
    // free srv and srv thread
    if (srv->_srv_t[i]) {
      mjtcpsrv_set_stop(srv->_srv[i], true);
      mjthread_delete(srv->_srv_t[i]);
    }
    if (srv->_srv_n[i]) mjsock_close(srv->_srv_n[i]);
  }
  // free worker threadpool
  if (srv->_worker_pool) mjthreadpool_delete(srv->_worker_pool);
  free(srv);
  return true;
}

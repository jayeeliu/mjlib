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

struct asy_data {
  int     _n_r;   // notify read side
  int     _n_w;   // notify write side
  mjev    _ev;    // event loop
  mjProc  _RT;    // thread routine
  void*   _rarg;  // thread routine arg
  mjProc  _CB;    // callback when thread routine finished
  void*   _carg;  // callback arg
};
typedef struct asy_data* asy_data;

/*
===============================================================================
mjmainsrv_asy_fin(server routine)
  call when asyncroutine finish
===============================================================================
*/
static void* mjmainsrv_asy_fin(void* data) {
  asy_data asy_d = (asy_data) data;
  // get and clean
  char buffer[2];
  read(asy_d->_n_r, buffer, sizeof(buffer));
  // delete nodify event
  mjev_del_fevent(asy_d->_ev, asy_d->_n_r, MJEV_READABLE);
  close(asy_d->_n_r);
  close(asy_d->_n_w);
  // run callback proc
  asy_d->_CB(asy_d->_carg);
  free(asy_d);
  return NULL;
}

/*
===============================================================================
mjmainsrv_asy_rt(thread routine)
  aync run routine
===============================================================================
*/
static void* mjmainsrv_asy_rt(void* data) {
  // get data from asyncData
  mjthread thread = (mjthread) data;
  asy_data asy_d = (asy_data) thread->arg;
  // run Routine
  asy_d->_RT(asy_d->_rarg);
  // notify eventloop
  write(asy_d->_n_w, "OK", 2);
  return NULL;
}

/*
===============================================================================
mjmainsrv_Async
  run workerRoutine in threadpool, when finish call CallBack
===============================================================================
*/
bool mjmainsrv_asy(mjtcpsrv srv, mjProc RT, void* rarg, mjProc CB, void* carg) {
  // sanity check
  if (!srv) return false;
  // alloc mjmainsrv_AsyncData
  asy_data asy_d = (asy_data) calloc(1, sizeof(struct asy_data));
  if (!asy_d) {
    MJLOG_ERR("AsycData alloc Error");
    return false;
  }
  asy_d->_ev    = mjtcpsrv_get_ev(srv);
  asy_d->_RT    = RT;
  asy_d->_rarg  = rarg;
  asy_d->_CB    = CB;
  asy_d->_carg  = carg;
  // alloc notify pipe and add callback event to eventloop
  int n_fd[2];
  if (pipe(n_fd)) {
    MJLOG_ERR("pipe alloc error: %s", strerror(errno));
    goto failout1;
  }
  asy_d->_n_r = n_fd[0];
  asy_d->_n_w = n_fd[1];
  mjev_add_fevent(asy_d->_ev, n_fd[0], MJEV_READABLE, mjmainsrv_asy_fin, asy_d);
  // add routine to threadpool
  mjmainsrv msrv = (mjmainsrv) mjtcpsrv_get_obj(srv, "mainsrv");
  if (!mjthreadpool_add_routine_plus(msrv->_wpool, mjmainsrv_asy_rt, asy_d)) {
    MJLOG_ERR("Oops async run Error");
    // del notify event
    mjev_del_fevent(asy_d->_ev, n_fd[0], MJEV_READABLE);
    goto failout2;
  }
  return true;

failout2:
  close(n_fd[0]);
  close(n_fd[1]);
failout1:
  free(asy_d);
  return false;
}

/*
===============================================================================
mjmainsrv_srv_run
  wrap function for srvthread_run
===============================================================================
*/
static void* mjmainsrv_is_run(void* arg) {
  mjthread thread = (mjthread) arg;
  mjtcpsrv srv = mjthread_get_obj(thread, "tcpserver");
  mjtcpsrv_run(srv);
  return NULL;
}

/*
===============================================================================
mjmainsrv_Run
  run main srv. just for accept and dispatch
===============================================================================
*/
bool mjmainsrv_run(mjmainsrv srv) {
  // sanity check
  if (!srv) return false;
  // create inner server thread
  for (int i = 0; i < srv->_is_num; i++) {
    srv->_is_t[i] = mjthread_new(NULL, NULL);
    if (!srv->_is_t[i]) {
      MJLOG_ERR("mjthread create error");
      return false;
    }
    mjthread_set_obj(srv->_is_t[i], "tcpserver", srv->_is[i], mjtcpsrv_delete);
    mjthread_add_routine(srv->_is_t[i], mjmainsrv_is_run, NULL);
  }
  // create threadpool
  srv->_wpool = mjthreadpool_new(srv->_is_num * 2, NULL, NULL); 
  if (!srv->_wpool) {
    MJLOG_ERR("threadpool create error");
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
    dispatch = (dispatch + 1) % srv->_is_num;
    if (write(srv->_is_n[dispatch], &cfd, sizeof(int)) < 0) {
      MJLOG_ERR("set socket to thread error, close");
      mjsock_close(cfd);
    }
  }
  return true;
}

/*
===============================================================================
mjmainsrv_New
  create new mjsrv struct
===============================================================================
*/
mjmainsrv mjmainsrv_new(int sfd, mjProc ISRT) {
  mjmainsrv srv = (mjmainsrv)calloc(1, sizeof(struct mjmainsrv));
  if (!srv) {
    MJLOG_ERR("mjsrv create error");
    return NULL;
  }
  mjsock_set_blocking(sfd, 1);
  // stage1: update server number
  srv->_sfd     = sfd;
  srv->_is_num  = get_cpu_count() - 1;
  if (srv->_is_num <= 0) {
    MJLOG_ERR("cpu count error");
    free(srv);
    return NULL;
  }
  // stage2: create inner server
  int fd[2];
  for (int i = 0; i < srv->_is_num; i++) {
    // set srvNotify
    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, fd)) {
      MJLOG_ERR("socketpair error");
      mjmainsrv_delete(srv);
      return NULL;
    }
    srv->_is_n[i] = fd[0];
    // create new srv struct and set main srv
    srv->_is[i] = mjtcpsrv_new(fd[1], ISRT, MJTCPSRV_INNER);
    if (!srv->_is[i]) {
      MJLOG_ERR("mjtcpsrv create error");
      mjmainsrv_delete(srv);
      return NULL;
    }
    mjtcpsrv_set_obj(srv->_is[i], "mainsrv", srv, NULL);
  }
  return srv;
}

/*
===============================================================================
mjmainsrv_Delete
  delete mjmainsrv struct
===============================================================================
*/
bool mjmainsrv_delete(mjmainsrv msrv) {
  // sanity check
  if (!msrv) return false;
  // free fd and mjTcpSrv
  for (int i = 0; i < msrv->_is_num; i++) {
    // free srv and srv thread
    if (msrv->_is_t[i]) {
      mjtcpsrv_set_stop(msrv->_is[i], true);
      mjthread_delete(msrv->_is_t[i]);
    } else if (msrv->_is[i]) {
      mjtcpsrv_delete(msrv->_is[i]);
    }
    if (msrv->_is_n[i]) mjsock_close(msrv->_is_n[i]);
  }
  // free worker threadpool
  if (msrv->_wpool) mjthreadpool_delete(msrv->_wpool);
  free(msrv);
  return true;
}

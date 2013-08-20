#define _GNU_SOURCE
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "mjmainsrv.h"
#include "mjconn.h"
#include "mjcomm.h"
#include "mjsock.h"
#include "mjsig.h"
#include "mjlog.h"

struct mjmainsrv_async_data {
  int     _fin_notify_r;
  int     _fin_notify_w;
  mjev    _ev;
  mjProc  _WorkerRoutine;
  void*   _w_arg;
  mjProc  _CallBack;
  void*   _c_arg;
};
typedef struct mjmainsrv_async_data* mjmainsrv_async_data;

/*
===============================================================================
mjmainsrv_AsyncFinCallBack
  call when asyncroutine finish
===============================================================================
*/
static void* mjmainsrv_async_fin_callback(void* data) {
  mjmainsrv_async_data async_data = (mjmainsrv_async_data) data;
  int fin_notify_r 	= async_data->_fin_notify_r;
  int fin_notify_w 	= async_data->_fin_notify_w;
  mjev ev         	= async_data->_ev;
  mjProc CallBack 	= async_data->_CallBack;
  void* c_arg     	= async_data->_c_arg;
  char buffer[2];
  // get and clean
  read(fin_notify_r, buffer, sizeof(buffer));
  mjev_del_fevent(ev, fin_notify_r, MJEV_READABLE);
  close(fin_notify_r);
  close(fin_notify_w);
  free(async_data);
  // run callback proc
  CallBack(c_arg);
  return NULL;
}

/*
===============================================================================
mjmainsrv_async_routine
  aync run routine
===============================================================================
*/
static void* mjmainsrv_async_routine(void* data) {
  // get data from asyncData
  mjthread thread = (mjthread) data;
  mjmainsrv_async_data async_data = (mjmainsrv_async_data) thread->arg;
  int fin_notify_w       = async_data->_fin_notify_w;
  mjProc WorkerRoutine  = async_data->_WorkerRoutine;
  void* w_arg           = async_data->_w_arg;
  // run Routine
  WorkerRoutine(w_arg);
  // notify eventloop
  write(fin_notify_w, "OK", 2);
  return NULL;
}

/*
===============================================================================
mjmainsrv_Async
  run workerRoutine in threadpool, when finish call CallBack
===============================================================================
*/
bool mjmainsrv_async(mjtcpsrv srv, mjProc WorkerRoutine, void* w_arg,
      mjProc CallBack, void* c_arg) {
  // sanity check
  if (!srv) {
    MJLOG_ERR("srv is null");
    return false;
  }
  mjmainsrv mainSrv = (mjmainsrv) mjtcpsrv_get_obj(srv, "mainSrv");
  // alloc mjmainsrv_AsyncData
  mjmainsrv_async_data async_data = (mjmainsrv_async_data) calloc(1,
        sizeof(struct mjmainsrv_async_data));
  if (!async_data) {
    MJLOG_ERR("AsycData alloc Error");
    return false;
  }
  async_data->_ev             = srv->ev;
  async_data->_WorkerRoutine  = WorkerRoutine;
  async_data->_w_arg          = w_arg;
  async_data->_CallBack       = CallBack;
  async_data->_c_arg          = c_arg;
  // alloc notify pipe and add callback event to eventloop
  int notifyFd[2];
  if (pipe(notifyFd)) {
    MJLOG_ERR("pipe alloc error");
    free(async_data);
    return false;
  }
  mjev_add_fevent(srv->ev, notifyFd[0], MJEV_READABLE, 
      mjmainsrv_async_fin_callback, async_data);
  async_data->_fin_notify_r  = notifyFd[0];
  async_data->_fin_notify_w  = notifyFd[1];
  // add routine to threadpool
  if (!mjthreadpool_add_routine_plus(mainSrv->_worker_thread_pool, 
      mjmainsrv_async_routine, async_data)) {
    MJLOG_ERR("Oops async run Error");
    // del notify event
    mjev_del_fevent(srv->ev, notifyFd[0], MJEV_READABLE);
    close(notifyFd[0]);
    close(notifyFd[1]);
    free(async_data);
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
bool mjmainsrv_run(mjmainsrv mainSrv) {
  // sanity check
  if (!mainSrv) {
    MJLOG_ERR("srv is null");
    return false;
  }
  // accept and dispatch
  static int dispatchServer = 0;
  while (!mainSrv->_stop) {
    int cfd = mjsock_accept(mainSrv->_sfd);
    if (cfd < 0) {
      MJLOG_ERR("mjSock_Accept Error continue");
      continue;
    }
    dispatchServer = (dispatchServer + 1) % mainSrv->_srv_num;
    if (write(mainSrv->_srv_notify[dispatchServer], &cfd, sizeof(int)) < 0) {
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
mjmainsrv_srvthread_run
  wrap function for srvthread_run
===============================================================================
*/
static void* mjmainsrv_srvthread_run(void* arg) {
  mjthread thread = (mjthread) arg;
  mjtcpsrv srv = mjthread_get_obj(thread, "thread_local");
  mjtcpsrv_run(srv);
  return NULL;
}

/*
===============================================================================
mjmainsrv_srvthread_exit
  wrap function for srvthread_exit
===============================================================================
*/
static void* mjmainsrv_srvthread_free_local(void* arg) {
  mjtcpsrv srv = (mjtcpsrv) arg;
  mjtcpsrv_delete(srv);
  return NULL;
}

/*
===============================================================================
mjmainsrv_New
  create new mjsrv struct
===============================================================================
*/
mjmainsrv mjmainsrv_new(int sfd, mjProc SrvRoutine, mjProc InitSrv,
    void* init_arg, int worker_thread_num) {
  // alloc srv struct
  mjmainsrv mainSrv = (mjmainsrv) calloc(1, sizeof(struct mjmainsrv));
  if (!mainSrv) {
    MJLOG_ERR("mjsrv create error");
    return NULL;
  }
  // set listen socket blocking
  mjsock_set_blocking(sfd, 1);
  // update fileds and srv
  mainSrv->_sfd    	= sfd;
  mainSrv->_srv_num	= get_cpu_count();
  if (mainSrv->_srv_num <= 0) {
    MJLOG_ERR("cpu count error");
    free(mainSrv);
    return NULL;
  }
  // create srv
  int fd[2];
  for (int i = 0; i < mainSrv->_srv_num; i++) {
    // set srvNotify
    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, fd)) {
      MJLOG_ERR("socketpair error");
      mjmainsrv_delete(mainSrv);
      return NULL;
    }
    mainSrv->_srv_notify[i] = fd[0];
    // create new srv struct and set mainSrv
    mainSrv->_srv[i] = mjtcpsrv_new(fd[1], SrvRoutine, InitSrv, 
        init_arg, MJTCPSRV_INNER);
    if (!mainSrv->_srv[i]) {
      MJLOG_ERR("mjTcpSrv create error");
      mjmainsrv_delete(mainSrv);
      return NULL;
    }
		mjtcpsrv_set_obj(mainSrv->_srv[i], "mainSrv", mainSrv, NULL);
    // create new thread
    mainSrv->_srv_thread[i] = mjthread_new(NULL, NULL);
    if (!mainSrv->_srv_thread[i]) {
      MJLOG_ERR("mjThread create error");
      mjmainsrv_delete(mainSrv);
      return NULL;
    }
    mjthread_set_obj(mainSrv->_srv_thread[i], "thread_local", mainSrv->_srv[i],
        mjmainsrv_srvthread_free_local);
    mjthread_add_routine(mainSrv->_srv_thread[i], mjmainsrv_srvthread_run, 
				NULL);
  }
  // update threadpool
  mainSrv->_worker_thread_pool = mjthreadpool_new(worker_thread_num, NULL, 
			NULL); 
  if (!mainSrv->_worker_thread_pool) {
    MJLOG_ERR("threadpool create error");
    mjmainsrv_delete(mainSrv);
    return NULL;
  }
  return mainSrv;
}

/*
===============================================================================
mjmainsrv_Delete
  delete mjmainsrv struct
===============================================================================
*/
bool mjmainsrv_delete(mjmainsrv mainSrv) {
  // sanity check
  if (!mainSrv) {
    MJLOG_ERR("srv is null");
    return false;
  }
  // free fd and mjTcpSrv
  for (int i = 0; i < mainSrv->_srv_num; i++) {
    // free srv and srv thread
    if (mainSrv->_srv_thread[i]) {
      mjtcpsrv_set_stop(mainSrv->_srv[i], true);
      mjthread_delete(mainSrv->_srv_thread[i]);
    }
    if (mainSrv->_srv_notify[i]) mjsock_close(mainSrv->_srv_notify[i]);
  }
  // free worker threadpool
  if (mainSrv->_worker_thread_pool) {
    mjthreadpool_delete(mainSrv->_worker_thread_pool);
  }
  free(mainSrv);
  return true;
}

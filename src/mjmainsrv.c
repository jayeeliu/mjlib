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
  int     finNotify_r;
  int     finNotify_w;
  mjev    ev;
  mjProc  workerRoutine;
  void*   rdata;
  mjProc  CallBack;
  void*   cdata;
};
typedef struct mjmainsrv_async_data* mjmainsrv_async_data;

/*
===============================================================================
mjmainsrv_AsyncFinCallBack
  call when asyncroutine finish
===============================================================================
*/
static void* mjmainsrv_async_fin_callback(void* data) {
  mjmainsrv_async_data asyncData = (mjmainsrv_async_data) data;
  int finNotify_r = asyncData->finNotify_r;
  int finNotify_w = asyncData->finNotify_w;
  mjev ev         = asyncData->ev;
  mjProc CallBack = asyncData->CallBack;
  void* cdata     = asyncData->cdata;
  char buffer[2];
  // get and clean
  read(finNotify_r, buffer, sizeof(buffer));
  mjev_del_fevent(ev, finNotify_r, MJEV_READABLE);
  close(finNotify_r);
  close(finNotify_w);
  free(asyncData);
  // run callback proc
  CallBack(cdata);
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
  mjmainsrv_async_data asyncData = (mjmainsrv_async_data) thread->arg;
  int finNotify_w       = asyncData->finNotify_w;
  mjProc workerRoutine  = asyncData->workerRoutine;
  void* rdata           = asyncData->rdata;
  // run Routine
  workerRoutine(rdata);
  // notify eventloop
  write(finNotify_w, "OK", 2);
  return NULL;
}

/*
===============================================================================
mjmainsrv_Async
  run workerRoutine in threadpool, when finish call CallBack
===============================================================================
*/
bool mjmainsrv_async(mjtcpsrv srv, mjProc workerRoutine, void* rdata,
      mjProc CallBack, void* cdata) {
  // sanity check
  if (!srv) {
    MJLOG_ERR("srv is null");
    return false;
  }
  mjmainsrv mainSrv = (mjmainsrv) srv->mainSrv;
  // alloc mjmainsrv_AsyncData
  mjmainsrv_async_data asyncData = (mjmainsrv_async_data) calloc(1,
        sizeof(struct mjmainsrv_async_data));
  if (!asyncData) {
    MJLOG_ERR("AsycData alloc Error");
    return false;
  }
  asyncData->ev             = srv->ev;
  asyncData->workerRoutine  = workerRoutine;
  asyncData->rdata          = rdata;
  asyncData->CallBack       = CallBack;
  asyncData->cdata          = cdata;
  // alloc notify pipe and add callback event to eventloop
  int notifyFd[2];
  if (pipe(notifyFd)) {
    MJLOG_ERR("pipe alloc error");
    free(asyncData);
    return false;
  }
  mjev_add_fevent(srv->ev, notifyFd[0], MJEV_READABLE, 
      mjmainsrv_async_fin_callback, asyncData);
  asyncData->finNotify_r  = notifyFd[0];
  asyncData->finNotify_w  = notifyFd[1];
  // add routine to threadpool
  if (!mjthreadpool_add_routine_plus(mainSrv->workerThreadPool, 
      mjmainsrv_async_routine, asyncData)) {
    MJLOG_ERR("Oops async run Error");
    // del notify event
    mjev_del_fevent(srv->ev, notifyFd[0], MJEV_READABLE);
    close(notifyFd[0]);
    close(notifyFd[1]);
    free(asyncData);
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
  // set cpu affinity
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(0, &cpuset);
  pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
  // accept and dispatch
  static int dispatchServer = 0;
  while (!mainSrv->stop) {
    int cfd = mjsock_accept(mainSrv->sfd);
    if (cfd < 0) {
      MJLOG_ERR("mjSock_Accept Error continue");
      continue;
    }
    dispatchServer = (dispatchServer + 1) % mainSrv->srvNum;
    int ret = write(mainSrv->srvNotify[dispatchServer], &cfd, sizeof(int)); 
    if (ret < 0) {
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
bool mjmainsrv_set_stop(mjmainsrv srv, int value) {
  if (!srv) return false;
  srv->stop = (value == 0) ? 0 : 1;
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
  mjtcpsrv srv = mjmap_get_obj(thread->arg_map, "thread_local");
  mjtcpsrv_run(srv);
  return NULL;
}

/*
===============================================================================
mjmainsrv_srvthread_exit
  wrap function for srvthread_exit
===============================================================================
*/
static void* mjmainsrv_srvthread_exit(void* arg) {
  mjthread thread = (mjthread) arg;
  mjtcpsrv srv = mjmap_get_obj(thread->arg_map, "thread_local");
  mjtcpsrv_delete(srv);
  return NULL;
}

/*
===============================================================================
mjmainsrv_New
  create new mjsrv struct
===============================================================================
*/
mjmainsrv mjmainsrv_new(int sfd, mjProc srvRoutine, mjProc InitSrv,
    mjProc ExitSrv, int workerThreadNum) {
  // alloc srv struct
  mjmainsrv mainSrv = (mjmainsrv) calloc(1, sizeof(struct mjmainsrv));
  if (!mainSrv) {
    MJLOG_ERR("mjsrv create error");
    return NULL;
  }
  // set listen socket blocking
  mjsock_set_blocking(sfd, 1);
  // update fileds and srv
  mainSrv->sfd    = sfd;
  mainSrv->srvNum = get_cpu_count();
  if (mainSrv->srvNum <= 0) {
    MJLOG_ERR("cpu count error");
    free(mainSrv);
    return NULL;
  }
  // create srv
  int fd[2];
  for (int i = 0; i < mainSrv->srvNum; i++) {
    // set srvNotify
    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, fd)) {
      MJLOG_ERR("socketpair error");
      mjmainsrv_delete(mainSrv);
      return NULL;
    }
    mainSrv->srvNotify[i] = fd[0];
    // create new srv struct and set mainSrv
    mainSrv->srv[i] = mjtcpsrv_new(fd[1], srvRoutine, InitSrv, 
        ExitSrv, MJTCPSRV_INNER);
    if (!mainSrv->srv[i]) {
      MJLOG_ERR("mjTcpSrv create error");
      mjmainsrv_delete(mainSrv);
      return NULL;
    }
    mainSrv->srv[i]->mainSrv = mainSrv;
    // create new thread
    mainSrv->srvThread[i] = mjthread_new(NULL, NULL, mjmainsrv_srvthread_exit);
    if (!mainSrv->srvThread[i]) {
      MJLOG_ERR("mjThread create error");
      mjmainsrv_delete(mainSrv);
      return NULL;
    }
    mjthread_set_local(mainSrv->srvThread[i], mainSrv->srv[i]);
    mjthread_add_routine(mainSrv->srvThread[i], mjmainsrv_srvthread_run, NULL);
    // set cpu affinity
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(i, &cpuset);
    pthread_setaffinity_np(mainSrv->srvThread[i]->thread_id, 
        sizeof(cpu_set_t), &cpuset);
  }
  // update threadpool
  mainSrv->workerThreadPool = mjthreadpool_new(workerThreadNum, NULL, NULL, NULL); 
  if (!mainSrv->workerThreadPool) {
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
  for (int i = 0; i < mainSrv->srvNum; i++) {
    // free srv and srv thread
    if (mainSrv->srvThread[i]) {
      mjtcpsrv_set_stop(mainSrv->srv[i], 1);
      mjthread_delete(mainSrv->srvThread[i]);
    }
    if (mainSrv->srvNotify[i]) mjsock_close(mainSrv->srvNotify[i]);
  }
  // free worker threadpool
  if (mainSrv->workerThreadPool) {
    mjthreadpool_delete(mainSrv->workerThreadPool);
  }
  free(mainSrv);
  return true;
}

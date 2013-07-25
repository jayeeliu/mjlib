#include <stdlib.h>
#include "mjconnb.h"
#include "mjlog.h"
#include "mjlf.h"
#include "mjsock.h"
#include "mjsig.h"

/*
===============================================================================
mjLF_Routine
  Routine Run
===============================================================================
*/
static void* mjLF_Routine(void* arg) {
  mjLF srv = (mjLF) arg;
  int cfd;
  // leader run this
  while (1) {
    cfd = mjSock_Accept(srv->sfd);
    if (cfd < 0) continue;
    // choose a new leader
    if (mjThreadPool_AddWorkPlus(srv->tPool, mjLF_Routine, srv)) break;
    MJLOG_ERR("Oops No Leader, Too Bad, Close Connection!!!");
    close(cfd);
  }
  // change to worker
  if (!srv->Routine) {
    MJLOG_ERR("No Routine Set, Exit!");
    close(cfd);
    return NULL;
  }
  // create new conn 
  mjconnb conn = mjconnb_New(cfd);
  if (!conn) {
    MJLOG_ERR("create mjconnb error");
    close(cfd);
    return NULL;
  }
  mjconnb_SetServer(conn, srv);
  mjconnb_SetTimeout(conn, srv->readTimeout, srv->writeTimeout);
  // run server routine
  srv->Routine(conn);
  return NULL; 
}

/*
===============================================================================
mjLF_SetPrivate
  set server private data
===============================================================================
*/
bool mjLF_SetPrivate(mjLF srv, void* private, mjProc FreePrivate) {
  if (!srv) {
    MJLOG_ERR("srv is null");
    return false;
  }
  srv->private      = private;
  srv->FreePrivate  = FreePrivate;
  return true;
}

/*
===============================================================================
mjLF_SetStop
  set server stop
===============================================================================
*/
bool mjLF_SetStop(mjLF srv, int value) {
  if (!srv) {
    MJLOG_ERR("srv is null");
    return false;
  }
  srv->stop = (value == 0) ? 0 : 1;
  return true;
}

/*
===============================================================================
mjLF_SetTimeout
  set server timeout
===============================================================================
*/
bool mjLF_SetTimeout(mjLF srv, int readTimeout, int writeTimeout) {
  if (!srv) {
    MJLOG_ERR("srv is null");
    return false;
  }
  srv->readTimeout  = readTimeout;
  srv->writeTimeout = writeTimeout;
  return true;
}

/*
===============================================================================
mjLF_Run
  run leader follow server
===============================================================================
*/
void mjLF_Run(mjLF srv) {
  if (!srv) return;
  while (!srv->stop) {
    sleep(3);
    mjSig_ProcessQueue();
  }
}

/*
===============================================================================
mjLF_New
  create mjLF struct, create threadpool and run first leader
===============================================================================
*/
mjLF mjLF_New(int sfd, mjProc Routine, int maxThread) {
  // alloc mjLF struct
  mjLF srv = (mjLF) calloc(1, sizeof(struct mjLF));
  if (!srv) {
    MJLOG_ERR("server create errror");
    return NULL;
  }
  // init new pool
  srv->tPool = mjThreadPool_New(maxThread);
  if (!srv->tPool) {
    MJLOG_ERR("mjthreadpool create error");
    free(srv);
    return NULL;
  }
  // set server socket and routine
  srv->sfd      = sfd;
  srv->Routine  = Routine;
  // add new worker 
  if (!mjThreadPool_AddWork(srv->tPool, mjLF_Routine, srv)) {
    MJLOG_ERR("mjthreadpool addwork");
    mjThreadPool_Delete(srv->tPool);
    free(srv);
    return NULL;
  }
  // init signal
  mjSig_Init();
  mjSig_Register(SIGPIPE, SIG_IGN);
  return srv;
}

/*
===============================================================================
mjLF_Delete
  Delete server
===============================================================================
*/
bool mjLF_Delete(mjLF srv) {
  if (!srv) {
    MJLOG_ERR("server is null");
    return false;
  }
  if (srv->private && srv->FreePrivate) srv->FreePrivate(srv->private);
  // delete thread pool
  mjThreadPool_Delete(srv->tPool);
  free(srv);
  return true;
}

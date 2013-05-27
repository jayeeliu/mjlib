#include <stdlib.h>
#include <unistd.h>
#include "mjtcpsrv.h"
#include "mjconn.h"
#include "mjsig.h"
#include "mjsock.h"
#include "mjlog.h"

/*
===============================================================================
mjTcpSrv_AcceptRoutine
  accept routine
===============================================================================
*/
void* mjTcpSrv_AcceptRoutine(void* arg) {
  mjTcpSrv srv = (mjTcpSrv) arg;
  // read new client socket
  int cfd;
  if (srv->type == MJTCPSRV_STANDALONE) {
    // standalone mode, accept new socket
    cfd = mjSock_Accept(srv->sfd);
    if (cfd < 0) return NULL;
  } else if (srv->type == MJTCPSRV_INNER) {
    // innner mode, read new socket
    int ret = read(srv->sfd, &cfd, sizeof(int));
    if (ret < 0 || cfd < 0) {
      MJLOG_ERR("Too Bad, read socket error");
      return NULL;
    }
  } else {
    MJLOG_ERR("mjTcpSrv type error");
    return NULL;
  }
  // no server routine exit
  if (!srv->Routine) {
    MJLOG_ERR("no server Routine found");
    mjSock_Close(cfd);
    return NULL;
  }
  // create new mjconn
  mjConn conn = mjConn_New(srv->ev, cfd);
  if (!conn) {
    MJLOG_ERR("mjConn create error");
    mjSock_Close(cfd);
    return NULL;
  }
  mjConn_SetServer(conn, srv);
  srv->Routine(conn);
  return NULL;
}

/*
===============================================================================
mjTcpSrv_Run
  run mjTcpSrv
===============================================================================
*/
void* mjTcpSrv_Run(void* arg) {
  // sanity check
  if (!arg) {
    MJLOG_ERR("server is null");
    return NULL;
  }
  mjTcpSrv srv = (mjTcpSrv) arg;
  // call init Proc
  if (srv->InitSrv) srv->InitSrv(srv);
  // enter loop
  while (!srv->stop) {
    mjEV_Run(srv->ev);
    if (srv->type == MJTCPSRV_STANDALONE) mjSig_ProcessQueue();
  }
  return NULL;
}

/*
===============================================================================
mjTcpSrv_SetPrivate
  set private data and proc
===============================================================================
*/
bool mjTcpSrv_SetPrivate(mjTcpSrv srv, void* private, mjProc FreePrivate) {
  if (!srv) {
    MJLOG_ERR("server is null");
    return false;
  }
  srv->private      = private;
  srv->FreePrivate  = FreePrivate;
  return true;
}

/*
===============================================================================
mjTcpSrv_SetSrvProc
  set server init and exit proc. called when server begin and exit.
  srv is the parameter
===============================================================================
*/
bool mjTcpSrv_SetSrvProc(mjTcpSrv srv, mjProc InitSrv, mjProc ExitSrv) {
  // sanity check
  if (!srv) {
    MJLOG_ERR("server is null");
    return false;
  }
  srv->InitSrv = InitSrv;
  srv->ExitSrv = ExitSrv;
  return true;
}

/*
===============================================================================
mjTcpSrv_SetStop
  set mjTcpSrv stop
===============================================================================
*/
bool mjTcpSrv_SetStop(mjTcpSrv srv, int value) {
  if (!srv) {
    MJLOG_ERR("server is null");
    return false;
  }
  srv->stop = (value == 0) ? 0 : 1;
  return true;
}

/*
===============================================================================
mjTcpSrv_New
  alloc mjTcpSrv struct
===============================================================================
*/
mjTcpSrv mjTcpSrv_New(int sfd, mjProc Routine, int type) {
  // alloc mjTcpSrv struct
  mjTcpSrv srv = (mjTcpSrv) calloc (1, sizeof(struct mjTcpSrv));  
  if (!srv) {
    MJLOG_ERR("create server error");
    goto failout1;
  }
  // check type
  if (type != MJTCPSRV_STANDALONE && type != MJTCPSRV_INNER) {
    MJLOG_ERR("server type error");
    goto failout2;
  }
  // set sfd nonblock
  mjSock_SetBlocking(srv->sfd, 0);
  // set fields
  srv->sfd      = sfd;
  srv->type     = type;
  srv->Routine  = Routine;
  // set event Loop
  srv->ev = mjEV_New();
  if (!srv->ev) {
    MJLOG_ERR("create ev error");
    goto failout2;
  }
  // add read event
  if ((mjEV_Add(srv->ev, srv->sfd, MJEV_READABLE, 
            mjTcpSrv_AcceptRoutine, srv)) < 0) {
    MJLOG_ERR("mjev add error");
    goto failout3;
  }
  // set signal
  if (type == MJTCPSRV_STANDALONE) {
    mjSig_Init();
    mjSig_Register(SIGPIPE, SIG_IGN);
  }
  return srv;

failout3:
  mjEV_Delete(srv->ev);
failout2:
  free(srv);
failout1:
  mjSock_Close(sfd);
  return NULL; 
}

/*
===============================================================================
mjTcpSrv_Delete
  delete mjtcpsrv2 struct
===============================================================================
*/
void* mjTcpSrv_Delete(void* arg) {
  mjTcpSrv srv = (mjTcpSrv) arg;
  // sanity check
  if (!srv) {
    MJLOG_ERR("server is null");
    return NULL;
  }
  // call exit proc
  if (srv->ExitSrv) srv->ExitSrv(srv);
  // free private
  if (srv->private && srv->FreePrivate) srv->FreePrivate(srv->private);
  mjEV_Delete(srv->ev);
  mjSock_Close(srv->sfd);
  free(srv);
  return NULL;
}

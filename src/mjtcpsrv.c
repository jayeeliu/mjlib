#include <stdlib.h>
#include <unistd.h>
#include "mjtcpsrv.h"
#include "mjconn.h"
#include "mjsig.h"
#include "mjsock.h"
#include "mjlog.h"

/*
===============================================================================
mjtcpsrv_accept_routine
  accept routine
===============================================================================
*/
void* mjtcpsrv_accept_routine(void* arg) {
  mjtcpsrv srv = (mjtcpsrv) arg;
  // read new client socket
  int cfd;
  if (srv->type == MJTCPSRV_STANDALONE) {
    // standalone mode, accept new socket
    cfd = mjsock_accept(srv->sfd);
    if (cfd < 0) return NULL;
  } else if (srv->type == MJTCPSRV_INNER) {
    // innner mode, read new socket
    int ret = read(srv->sfd, &cfd, sizeof(int));
    if (ret < 0 || cfd < 0) {
      MJLOG_ERR("Too Bad, read socket error");
      return NULL;
    }
  } else {
    MJLOG_ERR("mjtcpsrv type error");
    return NULL;
  }
  // no server routine exit
  if (!srv->Routine) {
    MJLOG_ERR("no server Routine found");
    mjsock_close(cfd);
    return NULL;
  }
  // create new mjconn
  mjConn conn = mjConn_New(srv->ev, cfd);
  if (!conn) {
    MJLOG_ERR("mjConn create error");
    mjsock_close(cfd);
    return NULL;
  }
  mjConn_SetServer(conn, srv);
  srv->Routine(conn);
  return NULL;
}

/*
===============================================================================
mjtcpsrv_Run
  run mjtcpsrv
===============================================================================
*/
void* mjtcpsrv_run(void* arg) {
  // sanity check
  if (!arg) {
    MJLOG_ERR("server is null");
    return NULL;
  }
  mjtcpsrv srv = (mjtcpsrv) arg;
  // call init Proc
  if (srv->InitSrv) srv->InitSrv(srv);
  // enter loop
  while (!srv->stop) {
    mjev_run(srv->ev);
    if (srv->type == MJTCPSRV_STANDALONE) mjSig_ProcessQueue();
  }
  return NULL;
}

/*
===============================================================================
mjtcpsrv_SetPrivate
  set private data and proc
===============================================================================
*/
bool mjtcpsrv_set_private(mjtcpsrv srv, void* private, mjProc FreePrivate) {
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
mjtcpsrv_SetSrvProc
  set server init and exit proc. called when server begin and exit.
  srv is the parameter
===============================================================================
*/
bool mjtcpsrv_set_srvproc(mjtcpsrv srv, mjProc InitSrv, mjProc ExitSrv) {
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
mjtcpsrv_SetStop
  set mjtcpsrv stop
===============================================================================
*/
bool mjtcpsrv_set_stop(mjtcpsrv srv, int value) {
  if (!srv) {
    MJLOG_ERR("server is null");
    return false;
  }
  srv->stop = (value == 0) ? 0 : 1;
  return true;
}

/*
===============================================================================
mjtcpsrv_New
  alloc mjtcpsrv struct
===============================================================================
*/
mjtcpsrv mjtcpsrv_new(int sfd, mjProc Routine, int type) {
  // alloc mjtcpsrv struct
  mjtcpsrv srv = (mjtcpsrv) calloc (1, sizeof(struct mjtcpsrv));  
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
  mjsock_set_blocking(srv->sfd, 0);
  // set fields
  srv->sfd      = sfd;
  srv->type     = type;
  srv->Routine  = Routine;
  // set event Loop
  srv->ev = mjev_new();
  if (!srv->ev) {
    MJLOG_ERR("create ev error");
    goto failout2;
  }
  // add read event
  if ((mjev_add_fevent(srv->ev, srv->sfd, MJEV_READABLE, 
          mjtcpsrv_accept_routine, srv)) < 0) {
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
  mjev_delete(srv->ev);
failout2:
  free(srv);
failout1:
  mjsock_close(sfd);
  return NULL; 
}

/*
===============================================================================
mjtcpsrv_Delete
  delete mjtcpsrv2 struct
===============================================================================
*/
void* mjtcpsrv_delete(void* arg) {
  mjtcpsrv srv = (mjtcpsrv) arg;
  // sanity check
  if (!srv) {
    MJLOG_ERR("server is null");
    return NULL;
  }
  // call exit proc
  if (srv->ExitSrv) srv->ExitSrv(srv);
  // free private
  if (srv->private && srv->FreePrivate) srv->FreePrivate(srv->private);
  mjev_delete(srv->ev);
  mjsock_close(srv->sfd);
  free(srv);
  return NULL;
}

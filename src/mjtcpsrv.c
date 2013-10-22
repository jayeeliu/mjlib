#include <stdlib.h>
#include <unistd.h>
#include "mjtcpsrv.h"
#include "mjconn.h"
#include "mjsig.h"
#include "mjsock.h"
#include "mjlog.h"

/*
===============================================================================
mjtcpsrv_accept_routine(server routine)
  accept routine
===============================================================================
*/
static void* mjtcpsrv_accept_routine(void* arg) {
  mjtcpsrv srv = (mjtcpsrv) arg;
  // read new client socket
  int cfd;
  if (srv->_type == MJTCPSRV_STANDALONE) { // STANDALONE
    // standalone mode, accept new socket
    cfd = mjsock_accept(srv->_sfd);
    if (cfd < 0) {
      MJLOG_ERR("accept error");
      return NULL; 
    }
  } else { // INNER
    // innner mode, read new socket
    int ret = read(srv->_sfd, &cfd, sizeof(int));
    if (ret < 0 || cfd < 0) {
      MJLOG_ERR("Too Bad, read socket error");
      return NULL;
    }
  }
  // no server routine exit
  if (!srv->_RT) {
    MJLOG_ERR("no server Routine found");
    mjsock_close(cfd);
    return NULL;
  }
  // create new mjconn
  mjconn conn = mjconn_new(srv->_ev, cfd);
  if (!conn) {
    MJLOG_ERR("mjConn create error");
    mjsock_close(cfd);
    return NULL;
  }
  mjconn_set_obj(conn, "server", srv, NULL);
  srv->_RT(conn);
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
  mjtcpsrv srv = (mjtcpsrv) arg;
  if (!srv) return NULL;
  // run init
  if (srv->_INIT) srv->_INIT(srv);
  // enter loop
  while (!srv->_stop) {
    mjev_run(srv->_ev);
    if (srv->_type == MJTCPSRV_STANDALONE) mjsig_process_queue();
  }
  return NULL;
}

/*
===============================================================================
mjtcpsrv_new
  alloc mjtcpsrv struct
===============================================================================
*/
mjtcpsrv mjtcpsrv_new(int sfd, int type) {
  // alloc mjtcpsrv struct
  mjtcpsrv srv = (mjtcpsrv) calloc(1, sizeof(struct mjtcpsrv));  
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
  mjsock_set_blocking(sfd, 0);
  // set fields
  srv->_sfd   = sfd;
  srv->_type  = type;
  // set _map
  srv->_map = mjmap_new(31);
  if (!srv->_map) {
    MJLOG_ERR("mjmap_new error");
    goto failout2;
  }
  // set event Loop
  srv->_ev = mjev_new();
  if (!srv->_ev) {
    MJLOG_ERR("create ev error");
    goto failout2;
  }
  // add read event
  if ((mjev_add_fevent(srv->_ev, srv->_sfd, MJEV_READABLE, 
          mjtcpsrv_accept_routine, srv)) < 0) {
    MJLOG_ERR("mjev add error");
    goto failout3;
  }
  // set signal
  if (type == MJTCPSRV_STANDALONE) {
    mjsig_init();
    mjsig_register(SIGPIPE, SIG_IGN);
  }
  return srv;

failout3:
  mjev_delete(srv->_ev);
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
  // sanity check
  mjtcpsrv srv = (mjtcpsrv) arg;
  if (!srv) return NULL;
  // call exit proc
  mjev_delete(srv->_ev);
  mjmap_delete(srv->_map);
  mjsock_close(srv->_sfd);
  free(srv);
  return NULL;
}

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
  if (srv->_type == MJTCPSRV_STANDALONE) {
    // standalone mode, accept new socket
    cfd = mjsock_accept(srv->_sfd);
    if (cfd < 0) return NULL; 
  } else if (srv->_type == MJTCPSRV_INNER) {
    // innner mode, read new socket
    int ret = read(srv->_sfd, &cfd, sizeof(int));
    if (ret < 0 || cfd < 0) {
      MJLOG_ERR("Too Bad, read socket error");
      return NULL;
    }
  } else {
    MJLOG_ERR("mjtcpsrv type error");
    return NULL;
  }
  // no server routine exit
  if (!srv->_Routine) {
    MJLOG_ERR("no server Routine found");
    mjsock_close(cfd);
    return NULL;
  }
  // create new mjconn
  mjconn conn = mjconn_new(srv->ev, cfd);
  if (!conn) {
    MJLOG_ERR("mjConn create error");
    mjsock_close(cfd);
    return NULL;
  }
  mjconn_set_obj(conn, "server", srv, NULL);
  mjconn_set_timeout(conn, 1000, 1000);
  srv->_Routine(conn);
  return NULL;
}

/*
===============================================================================
mjtcpsrv_Run
  run mjtcpsrv
===============================================================================
*/
void* mjtcpsrv_run(void* arg) {
  mjtcpsrv srv = (mjtcpsrv) arg;
  // sanity check
  if (!srv) {
    MJLOG_ERR("server is null");
    return NULL;
  }
  // call init Proc
  if (srv->_InitSrv) srv->_InitSrv(srv);
  // enter loop
  while (!srv->_stop) {
    mjev_run(srv->ev);
    if (srv->_type == MJTCPSRV_STANDALONE) mjsig_process_queue();
  }
  return NULL;
}

/*
===============================================================================
mjtcpsrv_SetStop
  set mjtcpsrv stop
===============================================================================
*/
bool mjtcpsrv_set_stop(mjtcpsrv srv, bool value) {
  if (!srv) {
    MJLOG_ERR("server is null");
    return false;
  }
  srv->_stop = value ? true : false;
  return true;
}

/*
===============================================================================
mjtcpsrv_get_obj
	get obj from tcpsrv
===============================================================================
*/
void* mjtcpsrv_get_obj(mjtcpsrv srv, const char* key) {
	if (!srv || !key) {
		MJLOG_ERR("srv or key is null");
		return NULL;
	}
	return mjmap_get_obj(srv->_arg_map, key);
}

/*
===============================================================================
mjtcpsrv_set_obj
	set obj to mjtcpsrv
===============================================================================
*/
bool mjtcpsrv_set_obj(mjtcpsrv srv, const char* key, void* obj, 
		mjProc obj_free) {
	if (!srv || !key) {
		MJLOG_ERR("srv or key is null");
		return false;
	}
	if (mjmap_set_obj(srv->_arg_map, key, obj, obj_free) < 0) return false;
	return true;
}

/*
===============================================================================
mjtcpsrv_New
  alloc mjtcpsrv struct
===============================================================================
*/
mjtcpsrv mjtcpsrv_new(int sfd, mjProc Routine, mjProc InitSrv, void* init_arg,
    int type) {
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
  srv->_sfd     = sfd;
  srv->_type    = type;
  srv->_Routine = Routine;
  srv->_InitSrv = InitSrv;
  srv->init_arg = init_arg;
  // set _arg_map
  srv->_arg_map = mjmap_new(31);
  if (!srv->_arg_map) {
    MJLOG_ERR("mjmap_new error");
    goto failout2;
  }
  // set event Loop
  srv->ev = mjev_new();
  if (!srv->ev) {
    MJLOG_ERR("create ev error");
    goto failout2;
  }
  // add read event
  if ((mjev_add_fevent(srv->ev, srv->_sfd, MJEV_READABLE, 
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
  mjev_delete(srv->ev);
  mjmap_delete(srv->_arg_map);
  mjsock_close(srv->_sfd);
  free(srv);
  return NULL;
}

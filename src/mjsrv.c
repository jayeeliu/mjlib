#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "mjsrv.h"
#include "mjconn.h"
#include "mjsig.h"
#include "mjsock.h"
#include "mjlog.h"

struct asyncProc {
  mjProc  proc;
  void*   arg;
};

struct async_data {
  int               _notify[2];   // notify pipe, 0 for read 1 for write
  mjev              _ev;
  struct asyncProc  _task;
  struct asyncProc  _callback;
};
typedef struct async_data* async_data;

/*
===============================================================================
mjsrv_accept_routine(server routine)
  accept routine
===============================================================================
*/
static void* mjsrv_accept_routine(void* arg) {
  mjsrv srv = arg;
  int cfd = mjsock_accept(srv->_sfd);
  if (cfd < 0) {
    if (errno == EAGAIN || errno == EINTR) return NULL;
     MJLOG_ERR("accept error %d", errno);
    return NULL; 
  }
  // no server routine exit
  if (!srv->_task) {
    MJLOG_ERR("No Server Task Found");
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
  srv->_task(conn);
  return NULL;
}

bool mjsrv_enable_listen(mjsrv srv) {
  if (!srv) return false;
  if (!mjev_add_fevent(srv->_ev, srv->_sfd, MJEV_READABLE, 
        mjsrv_accept_routine, srv)) {
    MJLOG_ERR("enable listen error");
    return false;
  }
  return true;
}

bool mjsrv_disable_listen(mjsrv srv) {
  if (!srv) return false;
  if (!mjev_del_fevent(srv->_ev, srv->_sfd, MJEV_READABLE)) {
    MJLOG_ERR("disable listen error");
    return false;
  }
  return true;
}

static void* mjsrv_async_fin(void* arg) {
  async_data data = arg;
  char buffer[2];
  read(data->_notify[0], buffer, sizeof(buffer));

  mjev_del_fevent(data->_ev, data->_notify[0], MJEV_READABLE);
  close(data->_notify[0]);
  close(data->_notify[1]);
  data->_callback.proc(data->_callback.arg);
  free(data);
  return NULL;
}

/*
===============================================================================
mjsrv_async_routine
===============================================================================
*/
static void* mjsrv_async_routine(mjthread thread, void* arg) {
  async_data data = arg;
  data->_task.proc(data->_task.arg);
  write(data->_notify[1], "OK", 2);
  return NULL;
}

/*
===============================================================================
mjsrv_async
===============================================================================
*/
bool mjsrv_async(mjsrv srv, mjProc proc, void* arg, 
    mjProc cbproc, void* cbarg) {
  if (!srv->_tpool) return false;
  async_data data = (async_data) calloc(1, sizeof(struct async_data));
  if (!data) {
    MJLOG_ERR("async_data alloc error");
    return false;
  }

  if (pipe(data->_notify)) {
    MJLOG_ERR("pipe error");
    free(data);
    return false;
  }
  data->_ev = srv->_ev;
  data->_task.proc = proc;
  data->_task.arg = arg;
  data->_callback.proc = cbproc;
  data->_callback.arg = cbarg;

  mjev_add_fevent(data->_ev, data->_notify[0], MJEV_READABLE, 
      mjsrv_async_fin, data);
  if (!mjthreadpool_set_task(srv->_tpool, mjsrv_async_routine, data)) {
    MJLOG_ERR("Oops async run error");
    mjev_del_fevent(data->_ev, data->_notify[0], MJEV_READABLE);
    close(data->_notify[0]);
    close(data->_notify[1]);
    free(data);
    return false;
  }
  return true;
}

bool mjsrv_set_tpool(mjsrv srv, int threadNum) {
  if (!srv) return false;
  srv->_tpool = mjthreadpool_new(threadNum);
  if (!srv->_tpool) {
    MJLOG_ERR("mjthreadpool_new error");
    return false;
  }
  return true;
}

/*
===============================================================================
mjsrv_run
  run mjsrv
===============================================================================
*/
bool mjsrv_run(mjsrv srv) {
  if (!srv) return NULL;
  if (srv->_init.proc) srv->_init.proc(srv, srv->_init.arg);
  if (srv->_tpool) mjthreadpool_run(srv->_tpool);
  mjsrv_enable_listen(srv);
  // enter loop
  while (!srv->_stop) {
    mjev_run(srv->_ev);
    mjsig_process_queue();
  }
  return NULL;
}

/*
===============================================================================
mjsrv_new
  alloc mjsrv struct
===============================================================================
*/
mjsrv mjsrv_new(int sfd) {
  // alloc mjsrv struct
  mjsrv srv = (mjsrv) calloc(1, sizeof(struct mjsrv));  
  if (!srv) {
    MJLOG_ERR("create server error");
    goto failout1;
  }
  mjsock_set_blocking(sfd, 0);
  // set fields
  srv->_sfd   = sfd;
  srv->_local = mjmap_new(31);
  if (!srv->_local) {
    MJLOG_ERR("mjmap_new error");
    goto failout2;
  }
  // set event Loop
  srv->_ev = mjev_new();
  if (!srv->_ev) {
    MJLOG_ERR("create ev error");
    goto failout3;
  }
  // set signal
  mjsig_init();
  mjsig_register(SIGPIPE, SIG_IGN);
  return srv;

failout3:
  mjmap_delete(srv->_local);
failout2:
  free(srv);
failout1:
  mjsock_close(sfd);
  return NULL; 
}

/*
===============================================================================
mjsrv_delete
===============================================================================
*/
bool mjsrv_delete(mjsrv srv) {
  if (!srv) return false;
  // call exit proc
  mjev_delete(srv->_ev);
  mjthreadpool_delete(srv->_tpool);
  mjmap_delete(srv->_local);
  mjsock_close(srv->_sfd);
  free(srv);
  return true;
}

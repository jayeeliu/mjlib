#include <stdlib.h>
#include "mjconnb.h"
#include "mjlog.h"
#include "mjlf.h"
#include "mjsock.h"
#include "mjsig.h"

/*
===============================================================================
mjlf_routine
  Routine Run
===============================================================================
*/
static void* mjlf_routine(void* arg) {
  mjthread thread = (mjthread) arg;
  mjlf srv = (mjlf) thread->arg;
  int cfd;
  // leader run this
  while (1) {
    cfd = mjsock_accept(srv->_sfd);
    if (cfd < 0) continue;
    // choose new leader
    if (mjthreadpool_add_routine_plus(srv->_tpool, mjlf_routine, srv)) break;
    MJLOG_ERR("Oops No Leader, Too Bad, Close Connection!!!");
    close(cfd);
  }
  // change to worker
  if (!srv->_Routine) {
    MJLOG_ERR("No Routine Set, Exit!");
    close(cfd);
    return NULL;
  }
  // create new conn 
  mjconnb conn = mjconnb_new(cfd);
  if (!conn) {
    MJLOG_ERR("create mjconnb error");
    close(cfd);
    return NULL;
  }
  mjconnb_set_server(conn, srv);
  mjconnb_set_shared(conn, thread);
  mjconnb_set_timeout(conn, srv->_read_timeout, srv->_write_timeout);
  // run server routine
  srv->_Routine(conn);
  mjconnb_delete(conn);
  return NULL; 
}

/*
===============================================================================
mjlf_SetStop
  set server stop
===============================================================================
*/
bool mjlf_set_stop(mjlf srv, bool value) {
  if (!srv) {
    MJLOG_ERR("srv is null");
    return false;
  }
  srv->_stop = value;
  return true;
}

/*
===============================================================================
mjlf_SetTimeout
  set server timeout
===============================================================================
*/
bool mjlf_set_timeout(mjlf srv, int read_timeout, int write_timeout) {
  if (!srv) {
    MJLOG_ERR("srv is null");
    return false;
  }
  srv->_read_timeout  = read_timeout;
  srv->_write_timeout = write_timeout;
  return true;
}

/*
===============================================================================
mjlf_Run
  run leader follow server
===============================================================================
*/
void mjlf_run(mjlf srv) {
  if (!srv) return;
  while (!srv->_stop) {
    sleep(3);
    mjsig_process_queue();
  }
}

/*
===============================================================================
mjlf_New
  create mjlf struct, create threadpool and run first leader
===============================================================================
*/
mjlf mjlf_new(int sfd, mjProc Routine, int max_thread, mjProc Init, 
    void* init_arg) {
  // alloc mjlf struct
  mjlf srv = (mjlf) calloc(1, sizeof(struct mjlf));
  if (!srv) {
    MJLOG_ERR("server create errror");
    return NULL;
  }
  // init new pool
  srv->_tpool = mjthreadpool_new(max_thread, Init, init_arg);
  if (!srv->_tpool) {
    MJLOG_ERR("mjthreadpool create error");
    free(srv);
    return NULL;
  }
  // set server socket and routine
  srv->_sfd      = sfd;
  srv->_Routine  = Routine;
  // add new worker 
  if (!mjthreadpool_add_routine(srv->_tpool, mjlf_routine, srv)) {
    MJLOG_ERR("mjthreadpool addwork");
    mjthreadpool_delete(srv->_tpool);
    free(srv);
    return NULL;
  }
  // init signal
  mjsig_init();
  mjsig_register(SIGPIPE, SIG_IGN);
  return srv;
}

/*
===============================================================================
mjlf_Delete
  Delete server
===============================================================================
*/
bool mjlf_delete(mjlf srv) {
  if (!srv) {
    MJLOG_ERR("server is null");
    return false;
  }
  // delete thread pool
  mjthreadpool_delete(srv->_tpool);
  free(srv);
  return true;
}

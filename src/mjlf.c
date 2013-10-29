#include <stdlib.h>
#include "mjconnb.h"
#include "mjlog.h"
#include "mjlf.h"
#include "mjsock.h"
#include "mjsig.h"

/*
===============================================================================
mjlf_routine(thread_routine)
  Routine Run
===============================================================================
*/
static void* mjlf_routine(void* arg) {
  // get thread and srv
  mjthread thread = (mjthread) arg;
  mjlf srv = (mjlf) thread->arg;
  // leader run this
  int cfd;
  while (1) {
    cfd = mjsock_accept(srv->_sfd);
    if (cfd < 0) continue;
    if (srv->_stop) {
      mjsock_close(cfd);
      return NULL;
    }
    // choose new leader
    if (mjthreadpool_add_routine_plus(srv->_tpool, mjlf_routine, srv)) break;
    MJLOG_ERR("Oops No Leader, Too Bad, Close Connection!!!");
    mjsock_close(cfd);
  }
  // change to worker
  if (!srv->_RT) {
    MJLOG_ERR("No Routine Set, Exit!");
    mjsock_close(cfd);
    return NULL;
  }
  // create new conn 
  mjconnb conn = mjconnb_new(cfd);
  if (!conn) {
    MJLOG_ERR("create mjconnb error");
    mjsock_close(cfd);
    return NULL;
  }
  mjconnb_set_obj(conn, "server", srv, NULL);
  mjconnb_set_obj(conn, "thread", thread, NULL);
  //run server routine(conn routine)
  srv->_RT(conn);
  mjconnb_delete(conn);
  return NULL; 
}

/*
===============================================================================
mjlf_Run
  run leader follow server
===============================================================================
*/
void* mjlf_run(mjlf srv) {
  if (!srv) return NULL;
  if (srv->_INIT) srv->_INIT(srv);
  // init new pool
  mjthreadpool_run(srv->_tpool);
  // add new worker 
  if (!mjthreadpool_add_routine(srv->_tpool, mjlf_routine, srv)) {
    MJLOG_ERR("mjthreadpool add routine error");
    mjthreadpool_delete(srv->_tpool);
    return NULL;
  }
  while (!srv->_stop) {
    MJLOG_ERR("plus number: %d", srv->_tpool->_plus);
    sleep(3);
    mjsig_process_queue();
  }
  return NULL;
}

/*
===============================================================================
mjlf_New
  create mjlf struct, create threadpool and run first leader
===============================================================================
*/
mjlf mjlf_new(int sfd, int nthread) {
  mjlf srv = (mjlf) calloc(1, sizeof(struct mjlf));
  if (!srv) {
    MJLOG_ERR("server create errror");
    return NULL;
  }
  // set server socket and routine
  srv->_sfd = sfd;
  srv->_tpool = mjthreadpool_new(nthread);
  if (!srv->_tpool) {
    MJLOG_ERR("mjthreadpool errror");
    free(srv);
    return NULL;
  }
  srv->_map = mjmap_new(31);
  if (!srv->_map) {
    MJLOG_ERR("mjmap new error");
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
mjlf_delete
  Delete server
===============================================================================
*/
bool mjlf_delete(mjlf srv) {
  if (!srv) return false;
  mjthreadpool_delete(srv->_tpool);
  mjmap_delete(srv->_map);
  free(srv);
  return true;
}

#include "mjconb.h"
#include "mjlog.h"
#include "mjlf.h"
#include "mjsock.h"
#include "mjsig.h"
#include <stdlib.h>
#include <poll.h>

/*
===============================================================================
mjlf_routine(thread_routine, arg in thread->arg)
  Routine Run
===============================================================================
*/
static void* mjlf_routine(mjthread thread, void* arg) {
  mjlf srv = arg;
  int cfd;

  while (1) {
    cfd = mjsock_accept_timeout(srv->_sfd, 1000);
    if (cfd <= 0) {
      if (srv->_stop) return NULL;
      continue;
    }
    // check stop
    if (srv->_stop) {
      mjsock_close(cfd);
      return NULL;
    }
    // choose new leader
    if (mjthreadpool_set_task(srv->_tpool, mjlf_routine, srv)) break;
    MJLOG_ERR("Oops No Leader, Too Bad, Close Connection!!!");
    mjsock_close(cfd);
  }
  // change to worker
  if (!srv->_task) {
    MJLOG_ERR("No Task Set, Exit");
    mjsock_close(cfd);
    return NULL;
  }
  // create new conn 
  mjconb conn = mjconb_new(cfd);
  if (!conn) {
    MJLOG_ERR("create mjconb error");
    mjsock_close(cfd);
    return NULL;
  }
  if (srv->_rto || srv->_wto) {
    mjconb_set_timeout(conn, srv->_rto, srv->_wto);
  }
  // run task
  srv->_task(srv, thread, conn);
  mjconb_delete(conn);
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
  if (srv->_init.proc) srv->_init.proc(srv, srv->_init.arg);
  // init new pool
  mjthreadpool_run(srv->_tpool);
  // add new worker 
  if (!mjthreadpool_set_task(srv->_tpool, mjlf_routine, srv)) {
    MJLOG_ERR("mjthreadpool add routine error");
    mjthreadpool_delete(srv->_tpool);
    return NULL;
  }
  while (!srv->_stop) {
    sleep(3);
    mjsig_process_queue();
  }
  return NULL;
}

/*
===============================================================================
mjlf_new
  create mjlf struct, create threadpool and run first leader
===============================================================================
*/
mjlf mjlf_new(int sfd, int nthread) {
  mjlf srv = (mjlf) calloc(1, sizeof(struct mjlf));
  if (!srv) {
    MJLOG_ERR("server create errror");
    return NULL;
  }
  mjsock_set_blocking(sfd, 0);
  // set server socket and routine
  srv->_sfd = sfd;
  srv->_tpool = mjthreadpool_new(nthread);
  if (!srv->_tpool) {
    MJLOG_ERR("mjthreadpool errror");
    free(srv);
    return NULL;
  }
  srv->_local = mjmap_new(31);
  if (!srv->_local) {
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
  mjmap_delete(srv->_local);
  free(srv);
  return true;
}

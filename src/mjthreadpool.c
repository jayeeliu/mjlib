#include "mjthreadpool.h"
#include "mjlog.h"
#include <stdlib.h>

/*
===============================================================================
mjthreadpool_AddWork
  add worker to thread pool
  return: 0 --- success, -1 --- fail
===============================================================================
*/ 
bool mjthreadpool_add_routine(mjthreadpool tpool, mjProc RT, void* arg) { 
  // sanity check
  if (!tpool || !tpool->_running || tpool->_stop) {
    MJLOG_ERR("mjthread pool error ");
    return false;
  }
  // get free thread
  mjthread thread = mjlockless_pop(tpool->_freelist); 
  if (!thread) return false;
  if (thread->_working) MJLOG_ERR("Oops get working thread");
  // dispatch work to thread
  int ret = mjthread_add_routine(thread, RT, arg);
  if (!ret) MJLOG_ERR("Oops AddWork Error, Thread Lost");
  return ret;
}

/*
================================================================================
mjthreadpool_AddWorkPlus
  call mjthreadpool_AddWork, if failed, call mjThread_RunOnce
================================================================================
*/
bool mjthreadpool_add_routine_plus(mjthreadpool tpool, mjProc RT, void* arg) {
  if (!tpool || !tpool->_running || tpool->_stop) {
    MJLOG_ERR("mjthread pool error ");
    return false;
  }
  // call mjthreadpool_AddWork
  if (!mjthreadpool_add_routine(tpool, RT, arg)) {
    return mjthread_new_once(tpool->_INIT, tpool->iarg, RT, arg);
  }
  return true;
}

/*
===============================================================================
mjthreadpool_run
  run mjthreadpool
===============================================================================
*/
bool mjthreadpool_run(mjthreadpool tpool) {
  if (!tpool) return false;
  for (int i = 0; i < tpool->_nthread; i++) {
    mjthread_run(tpool->_threads[i]);
  }
  tpool->_running = true;
  return true;
}

/*
===============================================================================
mjthreadpool_New
  init new thread pool
  return: NOT NULL--- mjthreadpool struct, NULL --- fail
===============================================================================
*/
mjthreadpool mjthreadpool_new(int nthread) {
  // alloc threadpool struct
  mjthreadpool tpool = (mjthreadpool) calloc(1, sizeof(struct mjthreadpool) + 
      nthread * sizeof(struct mjthread));
  if (!tpool) {
    MJLOG_ERR("mjthreadpool alloc error");
    return NULL;
  }
  // init field
  tpool->_nthread = nthread;
  tpool->_freelist = mjlockless_new(nthread + 1);
  if (!tpool->_freelist) {
    MJLOG_ERR("mjlockless_new error");
    free(tpool);
    return NULL;
  }
  // create mjthread struct
  for (int i = 0; i < tpool->_nthread; i++) {
    tpool->_threads[i] = mjthread_new();
    if (!tpool->_threads[i]) {
      mjthreadpool_delete(tpool);
      return NULL;
    }
    mjthread_set_tpool(tpool->_threads[i], tpool); 
    mjlockless_push(tpool->_freelist, tpool->_threads[i]);
  }
  return tpool; 
} 

/* 
===============================================================================
mjthreadpool_delete
  destory thread pool
===============================================================================
*/
bool mjthreadpool_delete(mjthreadpool tpool) {
  // sanity check 
  if (!tpool) return false;
  tpool->_stop = true; 
  if (tpool->_running) {
    for (int i = 0; i < tpool->_nthread; i++) {
      mjthread_delete(tpool->_threads[i]);
    }
  }
  mjlockless_delete(tpool->_freelist);
  free(tpool);
  return true; 
} 

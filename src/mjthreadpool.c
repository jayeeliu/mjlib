#include "mjthreadpool2.h"
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
  if (!tpool || tpool->_stop) {
    MJLOG_ERR("mjthread pool is null or stopped");
    return false;
  }
  // get free thread
  mjthread thread = mjlockless_pop(tpool->_free_list); 
  if (!thread) return false;
  if (thread->_running) MJLOG_ERR("Oops get running thread");
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
  if (!tpool || tpool->_stop) {
    MJLOG_ERR("mjthread pool is null or stopped");
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
mjthreadpool_New
  init new thread pool
  return: NOT NULL--- mjthreadpool struct, NULL --- fail
===============================================================================
*/
mjthreadpool mjthreadpool_new(int nthread, mjProc INIT, void* iarg) {
  // alloc threadpool struct
  mjthreadpool tpool = (mjthreadpool) calloc(1, sizeof(struct mjthreadpool) + 
      nthread * sizeof(struct mjthread));
  if (!tpool) {
    MJLOG_ERR("mjthreadpool alloc error");
    return NULL;
  }
  // init field
  tpool->_nthread = nthread;
  tpool->_INIT = INIT;
  tpool->iarg = iarg;
  tpool->_free_list = mjlockless_new(nthread + 1);
  if (!tpool->_free_list) {
    MJLOG_ERR("mjlockless_new error");
    free(tpool);
    return NULL;
  }
  // init thread
  for (int i = 0; i < tpool->_nthread; i++) {
    tpool->_threads[i] = mjthread_new(INIT, iarg);
    mjthread_set_obj(tpool->_threads[i], "tpool", tpool, NULL); 
    mjlockless_push(tpool->_free_list, tpool->_threads[i]);
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
  // free all thread
  for (int i = 0; i < tpool->_nthread; i++) mjthread_delete(tpool->_threads[i]);
  mjlockless_delete(tpool->_free_list);
  free(tpool);
  return true; 
} 

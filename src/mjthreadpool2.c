#include <stdlib.h>
#include "mjthreadpool2.h"
#include "mjlog.h"

/*
===============================================================================
mjthreadpool_AddWork
  add worker to thread pool
  return: 0 --- success, -1 --- fail
===============================================================================
*/ 
bool mjthreadpool_add_routine(mjthreadpool tpool, mjProc Routine, void* arg) { 
  // sanity check
  if (!tpool) {
    MJLOG_ERR("mjthread pool is null");
    return false;
  }
  if (tpool->_shutdown) {
    MJLOG_WARNING("mjthreadpool is shutdown");
    return false;
  }
  // get free thread
  mjthreadentry entry = mjlockless_pop(tpool->_free_list); 
  if (!entry) return false;
  if (entry->_thread->_running) {
    MJLOG_ERR("Oops get running thread, %u, %u", tpool->_free_list->_head,
        tpool->_free_list->_tail);
  }
  // dispatch work to thread
  int ret = mjthread_add_routine(entry->_thread, Routine, arg);
  if (!ret) {
    MJLOG_ERR("Oops running %d", entry->_thread->_running);
    MJLOG_ERR("Oops AddWork Error, Thread Lost");
  }
  return ret;
}

/*
================================================================================
mjthreadpool_AddWorkPlus
  call mjthreadpool_AddWork, if failed, call mjThread_RunOnce
================================================================================
*/
bool mjthreadpool_add_routine_plus(mjthreadpool tpool, mjProc Routine, 
    void* arg) {
  if (!tpool) {
    MJLOG_ERR("mjthread pool is null");
    return false;
  }
  if (tpool->_shutdown) {
    MJLOG_ERR("mjthreadpool is shutdwon");
    return false;
  }
  // call mjthreadpool_AddWork
  if (!mjthreadpool_add_routine(tpool, Routine, arg)) {
    return mjthread_new_once(tpool->_Init, tpool->init_arg, Routine, arg);
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
mjthreadpool mjthreadpool_new(int max_thread, mjProc Init, void* init_arg) {
  // alloc threadpool struct
  mjthreadpool tpool = (mjthreadpool) calloc(1, sizeof(struct mjthreadpool) + 
      max_thread * sizeof(struct mjthreadentry));
  if (!tpool) {
    MJLOG_ERR("mjthreadpool alloc error");
    return NULL;
  }
  // init field
  tpool->_max_thread  = max_thread;
  tpool->_Init = Init;
  tpool->init_arg = init_arg;
  tpool->_free_list = mjlockless_new(max_thread + 1);
  if (!tpool->_free_list) {
    MJLOG_ERR("mjlockless_new error");
    free(tpool);
    return NULL;
  }
  // init thread
  for (int i = 0; i < tpool->_max_thread; i++) {
    tpool->_thread_entrys[i].tpool = tpool;
    mjlockless_push(tpool->_free_list, &tpool->_thread_entrys[i]);
    // create new thread
    tpool->_thread_entrys[i]._thread = mjthread_new(Init, init_arg);
    mjthread_set_obj(tpool->_thread_entrys[i]._thread, "entry", 
        &tpool->_thread_entrys[i], NULL);
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
  if (!tpool) {
    MJLOG_ERR("tpool is null");
    return false;
  }
  // can't call it twice
  if (tpool->_shutdown) return false;
  tpool->_shutdown = true; 
  // free all thread
  for (int i = 0; i < tpool->_max_thread; i++) {
    mjthread_delete(tpool->_thread_entrys[i]._thread);
  }
  mjlockless_delete(tpool->_free_list);
  free(tpool);
  return true; 
} 

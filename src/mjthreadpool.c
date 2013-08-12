#include <stdlib.h>
#include "mjthreadpool.h"
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
  if (tpool->shutdown) {
    MJLOG_WARNING("mjthreadpool is shutdown");
    return false;
  }
  // no free thread
  if (list_empty(&tpool->freelist)) return false;
  // get free thread
  pthread_mutex_lock(&tpool->freelist_lock); 
  mjthreadentry entry = list_first_entry(&tpool->freelist, 
          struct mjthreadentry, nodeList);
  if (entry) {
    list_del_init(&entry->nodeList);
  }
  pthread_mutex_unlock(&tpool->freelist_lock); 
  if (!entry) return false;
  // dispatch work to thread
  int ret = mjthread_add_routine(entry->thread, Routine, arg);
  if (!ret) {
    MJLOG_ERR("Oops AddWork Error, Thread Lost");
  }
  return ret;
}

/*
===============================================================================
mjthreadpool_AddWorkPlus
  call mjthreadpool_AddWork, if failed, call mjThread_RunOnce
===============================================================================
*/
bool mjthreadpool_add_routine_plus(mjthreadpool tpool, 
  mjProc Routine, void* arg) {
  // call mjthreadpool_AddWork
  if (!mjthreadpool_add_routine(tpool, Routine, arg)) {
    return mjthread_new_once(tpool->Init_Thread, tpool->init_arg,
      tpool->Exit_Thread, NULL, Routine, arg);
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
mjthreadpool mjthreadpool_new(int max_thread, mjProc Init_Thread, 
    void* init_arg, mjProc Exit_Thread) {
  // alloc threadpool struct
  mjthreadpool tpool = (mjthreadpool) calloc(1, sizeof(struct mjthreadpool) + 
      max_thread * sizeof(struct mjthreadentry));
  if (!tpool) {
    MJLOG_ERR("mjthreadpool alloc error");
    return NULL;
  }
  // init field
  tpool->max_thread  = max_thread;
  tpool->Init_Thread = Init_Thread;
  tpool->init_arg = init_arg;
  tpool->Exit_Thread = Exit_Thread;
  pthread_mutex_init(&tpool->freelist_lock, NULL); 
  INIT_LIST_HEAD(&tpool->freelist); 
  // init thread
  for (int i = 0; i < tpool->max_thread; i++) {
    tpool->thread_entrys[i].tpool = tpool;
    INIT_LIST_HEAD(&tpool->thread_entrys[i].nodeList);
    list_add_tail(&tpool->thread_entrys[i].nodeList, &tpool->freelist);
    // create new thread
    tpool->thread_entrys[i].thread = mjthread_new(Init_Thread, init_arg, Exit_Thread);
    tpool->thread_entrys[i].thread->entry = &tpool->thread_entrys[i];
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
  if (tpool->shutdown) return false;
  tpool->shutdown = true; 
  // free all thread
  for (int i = 0; i < tpool->max_thread; i++) {
    mjthread_delete(tpool->thread_entrys[i].thread);
  }
  pthread_mutex_destroy(&tpool->freelist_lock);
  free(tpool);
  return true; 
} 

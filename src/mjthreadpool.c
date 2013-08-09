#include <stdlib.h>
#include <assert.h>
#include "mjthreadpool.h"
#include "mjlog.h"

/*
===============================================================================
mjthreadpool_Routine
  run mjthreadpool
===============================================================================
*/
static void* mjthreadpool_routine(void* arg) {
  // get thread entry struct
  mjthread thread = (mjthread) arg;
  mjthreadentry entry = (mjthreadentry) thread->arg;
  // change args
  thread->arg = entry->arg;
  // call Routine
  entry->Routine(thread);
  entry->Routine = NULL;
  entry->arg = NULL;
  // add thread to free list
  pthread_mutex_lock(&entry->tpool->freelist_lock);
  list_add_tail(&entry->nodeList, &entry->tpool->freelist);
  entry->tpool->freenum++;
  pthread_mutex_unlock(&entry->tpool->freelist_lock); 
  return NULL;
}

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
    entry->Routine = Routine;
    entry->arg = arg;
    assert(entry->thread->running==false);
    tpool->freenum--;
  }
  pthread_mutex_unlock(&tpool->freelist_lock); 
  if (!entry) return false;
  // dispatch work to thread
  int ret = mjthread_add_routine(entry->thread, mjthreadpool_routine, entry);
  if (!ret) {
    MJLOG_ERR("Oops AddWork Error, Thread Lost %d", tpool->freenum);
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
    return mjthread_new_once(tpool->Thread_Init_Routine,
      tpool->Thread_Exit_Routine, NULL, Routine, arg);
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
mjthreadpool mjthreadpool_new(int max_thread, mjProc Init_Routine,
    mjProc Exit_Routine) {
  // alloc threadpool struct
  mjthreadpool tpool = (mjthreadpool) calloc(1, sizeof(struct mjthreadpool) + 
      max_thread * sizeof(struct mjthreadentry));
  if (!tpool) {
    MJLOG_ERR("mjthreadpool alloc error");
    return NULL;
  }
  // init field
  tpool->max_thread  = max_thread;
  tpool->freenum = max_thread;
  tpool->Thread_Init_Routine = Init_Routine;
  tpool->Thread_Exit_Routine = Exit_Routine;
  pthread_mutex_init(&tpool->freelist_lock, NULL); 
  INIT_LIST_HEAD(&tpool->freelist); 
  // init thread
  for (int i = 0; i < tpool->max_thread; i++) {
    tpool->thread_entrys[i].tpool = tpool;
    INIT_LIST_HEAD(&tpool->thread_entrys[i].nodeList);
    list_add_tail(&tpool->thread_entrys[i].nodeList, &tpool->freelist);
    // create new thread
    tpool->thread_entrys[i].thread = mjthread_new(Init_Routine, Exit_Routine);
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

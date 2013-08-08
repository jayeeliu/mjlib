#include <stdlib.h>
#include "mjthreadpool.h"
#include "mjlog.h"

/*
===============================================================================
mjThreadPool_Routine
  run mjthreadpool
===============================================================================
*/
static void* mjThreadPool_Routine(void* arg) {
  // get thread entry struct
  mjThreadEntry entry = (mjThreadEntry) arg;
  // call Routine
  entry->Routine(entry->arg);
  entry->Routine = NULL;
  entry->arg = NULL;
  // add thread to free list 
  pthread_mutex_lock(&entry->tPool->freelist_lock);
  list_add_tail(&entry->nodeList, &entry->tPool->freelist);
  pthread_mutex_unlock(&entry->tPool->freelist_lock); 
  return NULL;
}

/*
===============================================================================
mjThreadPool_AddWork
  add worker to thread pool
  return: 0 --- success, -1 --- fail
===============================================================================
*/ 
bool mjThreadPool_AddWork(mjThreadPool tPool, mjProc Routine, void* arg) { 
  // sanity check
  if (!tPool) {
    MJLOG_ERR("mjthread pool is null");
    return false;
  }
  if (tPool->shutdown) {
    MJLOG_WARNING("mjthreadpool is shutdown");
    return false;
  }
  // no free thread
  if (list_empty(&tPool->freelist)) return false;
  // get free thread
  pthread_mutex_lock(&tPool->freelist_lock); 
  mjThreadEntry entry  = list_first_entry(&tPool->freelist, 
          struct mjThreadEntry, nodeList);
  if (entry) {
    list_del_init(&entry->nodeList);
    entry->Routine = Routine;
    entry->arg = arg;
  }
  pthread_mutex_unlock(&tPool->freelist_lock); 
  if (!entry) return false;
  // dispatch work to thread
  int ret = mjThread_AddWork(entry->thread, mjThreadPool_Routine, entry);
  if (!ret) {
    MJLOG_ERR("Oops AddWork Error, Thread Lost");
  }
  return ret;
}

/*
===============================================================================
mjThreadPool_AddWorkPlus
  call mjThreadPool_AddWork, if failed, call mjThread_RunOnce
===============================================================================
*/
bool mjThreadPool_AddWorkPlus(mjThreadPool tPool, 
  mjProc Routine, void* arg) {
  // call mjThreadPool_AddWork
  if (!mjThreadPool_AddWork(tPool, Routine, arg)) {
    return mjThread_RunOnce(Routine, arg);
  }
  return true;
}

/*
===============================================================================
mjThreadPool_New
  init new thread pool
  return: NOT NULL--- mjThreadPool struct, NULL --- fail
===============================================================================
*/
mjThreadPool mjThreadPool_New(int max_thread, mjProc Init_Routine,
    mjProc Exit_Routine) {
  // alloc threadpool struct
  mjThreadPool tpool = (mjThreadPool) calloc(1, sizeof(struct mjThreadPool) + 
      max_thread * sizeof(struct mjThreadEntry));
  if (!tpool) {
    MJLOG_ERR("mjThreadPool alloc error");
    return NULL;
  }
  // init field
  tpool->max_thread  = max_thread;
  pthread_mutex_init(&tpool->freelist_lock, NULL); 
  INIT_LIST_HEAD(&tpool->freelist); 
  // init thread
  for (int i = 0; i < tpool->max_thread; i++) {
    tpool->threads_entry[i].tPool = tpool;
    INIT_LIST_HEAD(&tpool->threads_entry[i].nodeList);
    list_add_tail(&tpool->threads_entry[i].nodeList, &tpool->freelist);
    // create new thread
    tpool->threads_entry[i].thread = mjthread_new(Init_Routine, Exit_Routine, 
        NULL, &tpool->threads_entry[i]);
  }
  return tpool; 
} 

/* 
===============================================================================
mjThreadPool_delete
  destory thread pool
===============================================================================
*/
bool mjThreadPool_Delete(mjThreadPool tPool) {
  // sanity check 
  if (!tPool) {
    MJLOG_ERR("tPool is null");
    return false;
  }
  // can't call it twice
  if (tPool->shutdown) return false;
  tPool->shutdown = 1; 
  // free all thread
  for (int i = 0; i < tPool->max_thread; i++) {
    mjthread_delete(tPool->threads_entry[i].thread);
  }
  // free mutex
  pthread_mutex_destroy(&tPool->freelist_lock);
  // free memory
  free(tPool);
  return true; 
} 

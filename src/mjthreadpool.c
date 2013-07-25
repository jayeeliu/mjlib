#include <stdlib.h>
#include "mjthreadpool.h"
#include "mjlog.h"

/*
===============================================================================
mjThreadPool_ThreadFin
  thread post routine
  add thread to freelist
===============================================================================
*/
static void* mjThreadPool_ThreadFin(void* arg) {
  // get thread entry struct
  mjThread thread = (mjThread) arg;
  mjThreadEntry entry = (mjThreadEntry) thread->private;
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
  if (tPool->shutDown) {
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
  }
  pthread_mutex_unlock(&tPool->freelist_lock); 
  if (!entry) return false;
  // dispatch work to thread
  int ret = mjThread_AddWork(entry->thread, Routine, arg, NULL, NULL,
          mjThreadPool_ThreadFin, entry->thread);
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
mjThreadPool mjThreadPool_New(int maxThread) {
  // alloc threadpool struct
  mjThreadPool tPool = (mjThreadPool) calloc(1, sizeof(struct mjThreadPool) + 
      maxThread * sizeof(struct mjThreadEntry));
  if (!tPool) {
    MJLOG_ERR("mjThreadPool alloc error");
    return NULL;
  }
  // init field
  tPool->maxThread  = maxThread;
  pthread_mutex_init(&tPool->freelist_lock, NULL); 
  INIT_LIST_HEAD(&tPool->freelist); 
  // init thread
  for (int i = 0; i < tPool->maxThread; i++) {
    tPool->threads[i].tPool = tPool;
    INIT_LIST_HEAD(&tPool->threads[i].nodeList);
    list_add_tail(&tPool->threads[i].nodeList, &tPool->freelist);
    // create new thread
    tPool->threads[i].thread = mjthread_new();
    // set mjThreadEntry as private data
    mjthread_set_private(tPool->threads[i].thread, &tPool->threads[i], NULL);
  }
  return tPool; 
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
  if (tPool->shutDown) return false;
  tPool->shutDown = 1; 
  // free all thread
  for (int i = 0; i < tPool->maxThread; i++) {
    mjthread_delete(tPool->threads[i].thread);
  }
  // free mutex
  pthread_mutex_destroy(&tPool->freelist_lock);
  // free memory
  free(tPool);
  return true; 
} 

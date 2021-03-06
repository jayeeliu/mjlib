#include "mjthreadpool.h"
#include "mjlog.h"
#include <stdlib.h>

/*
===============================================================================
mjthreadpool_callback, thread routine
  mjthreadpool normal callback, put thread into freelist
===============================================================================
*/
static void* mjthreadpool_callback(mjthread thread, void* arg) {
  mjthreadpool tpool = arg;
  mjlockless_push(tpool->_freelist, thread);
  return NULL;
}

/*
===============================================================================
mjthreadpool_once_callback, thread routine
  mjthreadpool once callback, decrease plus count
===============================================================================
*/
static void* mjthreadpool_once_callback(mjthread thread, void* arg) {
  mjthreadpool tpool = arg;
  pthread_mutex_lock(&tpool->_pluslock);
  if (--tpool->_plus == 0) pthread_cond_signal(&tpool->_plusready);
  pthread_mutex_unlock(&tpool->_pluslock);
  return NULL;
}

/*
===============================================================================
mjthreadpool_add_task_real
  add worker to thread pool
===============================================================================
*/ 
static bool mjthreadpool_set_task_real(mjthreadpool tpool, mjThrdProc proc, 
    void* arg) { 
  mjthread thread = mjlockless_pop(tpool->_freelist); 
  if (!thread) return false;
  if (thread->_task.proc) MJLOG_ERR("Oops get working thread");
  // dispatch work to thread
  int ret = mjthread_set_task(thread, proc, arg);
  if (!ret) MJLOG_ERR("Oops AddWork Error, Thread Lost");
  return ret;
}

/*
================================================================================
mjthreadpool_add_task
================================================================================
*/
bool mjthreadpool_set_task(mjthreadpool tpool, mjThrdProc proc, void* arg) {
  if (!tpool || !tpool->_running || tpool->_stop || !proc) return false;

  if (!mjthreadpool_set_task_real(tpool, proc, arg)) {
    mjthread thread = mjthread_new();
    if (!thread) return false;
    mjthread_set_init(thread, tpool->_init.proc, tpool->_init.arg);
    mjthread_set_callback(thread, mjthreadpool_once_callback, tpool);
    pthread_mutex_lock(&tpool->_pluslock);
    tpool->_plus++;
    pthread_mutex_unlock(&tpool->_pluslock);
    return mjthread_run_once(thread, proc, arg);
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
  if (!tpool || tpool->_running) return false;

  for (int i = 0; i < tpool->_nthread; i++) {
    mjthread_set_init(tpool->_threads[i], tpool->_init.proc, tpool->_init.arg);
    mjthread_set_callback(tpool->_threads[i], mjthreadpool_callback, tpool);
    mjthread_run(tpool->_threads[i]);
  }
  tpool->_running = true;
  return true;
}

/*
===============================================================================
mjthreadpool_new
  init new thread pool
  return: NOT NULL--- mjthreadpool struct, NULL --- fail
===============================================================================
*/
mjthreadpool mjthreadpool_new(int nthread) {
  if (nthread <= 0) return NULL;

  mjthreadpool tpool = (mjthreadpool) calloc(1, sizeof(struct mjthreadpool) + 
      nthread * sizeof(struct mjthread));
  if (!tpool) {
    MJLOG_ERR("mjthreadpool alloc error");
    return NULL;
  }
  tpool->_nthread = nthread;
  tpool->_freelist = mjlockless_new(nthread + 1);
  if (!tpool->_freelist) {
    MJLOG_ERR("mjlockless_new error");
    free(tpool);
    return NULL;
  }
  // create mjthread, set normal cb
  for (int i = 0; i < tpool->_nthread; i++) {
    tpool->_threads[i] = mjthread_new();
    if (!tpool->_threads[i]) {
      mjthreadpool_delete(tpool);
      return NULL;
    }
    mjlockless_push(tpool->_freelist, tpool->_threads[i]);
  }
  // init plus
  pthread_mutex_init(&tpool->_pluslock, NULL);
  pthread_cond_init(&tpool->_plusready, NULL);
  return tpool; 
} 

/* 
===============================================================================
mjthreadpool_delete
  destory thread pool
===============================================================================
*/
bool mjthreadpool_delete(mjthreadpool tpool) {
  if (!tpool || tpool->_stop) return false;
  tpool->_stop = true; 
  if (tpool->_running) {
    for (int i = 0; i < tpool->_nthread; i++) {
      mjthread_delete(tpool->_threads[i]);
    }
  }
  // wait for once threads
  pthread_mutex_lock(&tpool->_pluslock);
  while (tpool->_plus) {
    pthread_cond_wait(&tpool->_plusready, &tpool->_pluslock);
  }
  pthread_mutex_unlock(&tpool->_pluslock);

  pthread_mutex_destroy(&tpool->_pluslock);
  pthread_cond_destroy(&tpool->_plusready);
  mjlockless_delete(tpool->_freelist);
  free(tpool);
  return true; 
} 

#include <stdlib.h>
#include <pthread.h>
#include "mjthread2.h"
#include "mjlog.h"
#include "mjthreadpool2.h"

/*
===============================================================================
mjthread_RunOnce
  create thread and run Routine
===============================================================================
*/
static void* mjthread_once_routine(void* arg) {
  // create and detach thread
  mjthread thread = (mjthread) arg;
  if (thread->_INIT) thread->_INIT(thread);
  if (thread->_RT) thread->_RT(thread);
  mjmap_delete(thread->_map);
  free(thread);
  return NULL;
}

/*
===============================================================================
mjthread_new_once
  create thread run once and exit
===============================================================================
*/
bool mjthread_new_once(mjProc INIT, void* iarg, mjProc RT, void* arg) {
  // alloc mjthread struct
  mjthread thread = (mjthread) calloc(1, sizeof(struct mjthread));
  if (!thread) {
    MJLOG_ERR("mjthread create error");
    return false;
  }
  thread->_INIT = INIT;
  thread->iarg = iarg;
  thread->_RT = RT;
  thread->arg = arg;
  // init arg_map
  thread->_map = mjmap_new(31);
  if (!thread->_map) {
    MJLOG_ERR("mjmap_new error");
    free(thread);
    return false;
  }
  // init thread
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&thread->_thread_id, &attr, mjthread_once_routine, thread);
  pthread_attr_destroy(&attr);
  return true;
}

/*
===============================================================================
ThreadRoutine:
  used for short caculate task
===============================================================================
*/
static void* mjthread_normal_routine(void* arg) {
  mjthread thread = (mjthread) arg;
  // call init Routine
  if (thread->_INIT) thread->_INIT(thread);
  // threadloop 
  while (1) {
    // wait for routine and not shutdown
    pthread_mutex_lock(&thread->_thread_lock);
    while (!thread->_running && !thread->_stop) {
      pthread_cond_wait(&thread->_thread_ready, &thread->_thread_lock);
    }
    pthread_mutex_unlock(&thread->_thread_lock);
    // call routine
    if (thread->_RT) thread->_RT(thread);
    // should shutdown, break
    if (thread->_stop) break;
    // clean for next task
    thread->_RT = NULL;
    thread->arg = NULL;
    thread->_running = false;
    // if in threadpool, add to freelist
    mjthreadpool tpool = mjmap_get_obj(thread->_map, "tpool");
    if (tpool) mjlockless_push(tpool->_free_list, thread);
  }
  thread->_closed = true;
  pthread_exit(NULL);
}

/*
===============================================================================
mjthread_AddWork
  add Routine to thread
===============================================================================
*/
bool mjthread_add_routine(mjthread thread, mjProc RT, void* arg) {
  // sanity check
  if (!thread || !RT) return false;
  // add worker to thread
  pthread_mutex_lock(&thread->_thread_lock);
  if (thread->_running) {
    mjthreadpool tpool = mjmap_get_obj(thread->_map, "tpool");
    if (tpool) MJLOG_ERR("Oops: thread is busy, can't happen in threadpool");
    pthread_mutex_unlock(&thread->_thread_lock);
    return false;
  }
  thread->_RT = RT;
  thread->arg = arg;
  thread->_running = true;
  pthread_cond_signal(&thread->_thread_ready);
  pthread_mutex_unlock(&thread->_thread_lock);
  return true;
}

/*
===============================================================================
mjthread_new
  create new thread, run mjthread_normal_routine
===============================================================================
*/
mjthread mjthread_new(mjProc INIT, void* iarg) {
  // alloc mjthread struct
  mjthread thread = (mjthread) calloc(1, sizeof(struct mjthread));
  if (!thread) {
    MJLOG_ERR("mjthread create error");
    return NULL;
  }
  thread->_INIT = INIT;
  thread->iarg = iarg;
  thread->_map = mjmap_new(31);
  if (!thread->_map) {
    MJLOG_ERR("mjmap_new error");
    free(thread);
    return NULL;
  }
  // init fields
  pthread_mutex_init(&thread->_thread_lock, NULL);
  pthread_cond_init(&thread->_thread_ready, NULL);
  pthread_create(&thread->_thread_id, NULL, mjthread_normal_routine, thread);
  return thread;
}

/*
===============================================================================
mjthread_delete
  stop thread
===============================================================================
*/
bool mjthread_delete(mjthread thread) {
  if (!thread) return false;
  thread->_stop = true;
  pthread_cond_broadcast(&thread->_thread_ready);
  // wait thread exit
  pthread_join(thread->_thread_id, NULL);
  // only normal thread need destory
  mjmap_delete(thread->_map);
  pthread_mutex_destroy(&thread->_thread_lock);
  pthread_cond_destroy(&thread->_thread_ready);
  free(thread);
  return true;
}

#include "mjlog.h"
#include "mjthread.h"
#include "mjthreadpool.h"
#include <stdlib.h>

#define MJTHREAD_FREE   0
#define MJTHREAD_NORMAL 1
#define MJTHREAD_ONCE   2

/*
===============================================================================
mjthread_once_routine(thread routine)
  create thread and run Routine
===============================================================================
*/
static void* mjthread_once_routine(void* arg) {
  // create and detach thread
  mjthread thread = (mjthread) arg;
  if (thread->_INIT) thread->_INIT(thread);
  if (thread->_RT) thread->_RT(thread);
  if (thread->_CB) thread->_CB(thread);
  return NULL;
}

/*
===============================================================================
mjthread_run_once
  mjthread run once
===============================================================================
*/
bool mjthread_run_once(mjthread thread, mjProc RT, void* arg) {
  if (!thread || thread->_type != MJTHREAD_FREE) return false;
  thread->_RT = RT;
  thread->arg = arg;
  thread->_type = MJTHREAD_ONCE;
  // thread run
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&thread->_id, &attr, mjthread_once_routine, thread);
  pthread_attr_destroy(&attr);
  return true;
}

/*
===============================================================================
mjthread_routine(thread routine)
  normal thread routine
===============================================================================
*/
static void* mjthread_routine(void* arg) {
  mjthread thread = (mjthread) arg;
  if (thread->_INIT) thread->_INIT(thread);
  while (1) {
    // wait for routine and not shutdown
    pthread_mutex_lock(&thread->_lock);
    while (!thread->_working && !thread->_stop) {
      pthread_cond_wait(&thread->_ready, &thread->_lock);
    }
    pthread_mutex_unlock(&thread->_lock);
    if (thread->_RT) thread->_RT(thread);
    // clean for next task
    thread->_RT = NULL;
    thread->arg = NULL;
    thread->_working = false;
    // callback run
    if (thread->_CB) thread->_CB(thread);
    // should stop, break
    if (thread->_stop) break;
  }
  pthread_exit(NULL);
}

/*
===============================================================================
mjthread_add_task
  add RT to thread, return false if thread is working
===============================================================================
*/
bool mjthread_add_task(mjthread thread, mjProc RT, void* arg) {
  if (!thread || thread->_type != MJTHREAD_NORMAL || !RT) return false;
  // add worker to thread
  pthread_mutex_lock(&thread->_lock);
  if (thread->_working) {
    pthread_mutex_unlock(&thread->_lock);
    return false;
  }
  thread->_RT = RT;
  thread->arg = arg;
  thread->_working = true;
  pthread_cond_signal(&thread->_ready);
  pthread_mutex_unlock(&thread->_lock);
  return true;
}

/*
===============================================================================
mjthread_run
  run mjthread
===============================================================================
*/
bool mjthread_run(mjthread thread) {
  if (!thread || thread->_type != MJTHREAD_FREE) return false;
  // init fields
  pthread_mutex_init(&thread->_lock, NULL);
  pthread_cond_init(&thread->_ready, NULL);
  pthread_create(&thread->_id, NULL, mjthread_routine, thread);
  thread->_type = MJTHREAD_NORMAL;
  return true;
}

/*
===============================================================================
mjthread_new
  create mjthread struct
===============================================================================
*/
mjthread mjthread_new() {
  mjthread thread = (mjthread) calloc(1, sizeof(struct mjthread));
  if (!thread) {
    MJLOG_ERR("mjthread create error");
    return NULL;
  }
  thread->_map = mjmap_new(31);
  if (!thread->_map) {
    MJLOG_ERR("mjmap_new error");
    free(thread);
    return NULL;
  }
  return thread;
}

/*
===============================================================================
mjthread_delete
  stop thread, once routine thread never call this
===============================================================================
*/
bool mjthread_delete(mjthread thread) {
  if (!thread || thread->_stop) return false;
  thread->_stop = true;
  if (thread->_type == MJTHREAD_NORMAL) {
    pthread_cond_broadcast(&thread->_ready);
    pthread_join(thread->_id, NULL);
    pthread_mutex_destroy(&thread->_lock);
    pthread_cond_destroy(&thread->_ready);
  } else if (thread->_type == MJTHREAD_ONCE) {
    pthread_join(thread->_id, NULL);
  }
  mjmap_delete(thread->_map);
  free(thread);
  return true;
}

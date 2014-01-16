#include "mjlog.h"
#include "mjthread.h"
#include <stdlib.h>

#define MJTHREAD_NOTRUN   0
#define MJTHREAD_NORMAL 1
#define MJTHREAD_ONCE   2

/*
===============================================================================
mjthread_once_routine(thread routine)
  create thread and run Routine, del self when finished
===============================================================================
*/
static void* mjthread_once_routine(void* arg) {
  mjthread thread = arg;
  if (thread->_init.proc) thread->_init.proc(thread, thread->_init.arg);
  if (thread->_task.proc) thread->_task.proc(thread, thread->_task.arg);
  if (thread->_callback.proc) {
    thread->_callback.proc(thread, thread->_callback.arg);
  }
  mjmap_delete(thread->_local);
  free(thread);
  pthread_exit(NULL);
}

/*
===============================================================================
mjthread_run_once
  mjthread run once
===============================================================================
*/
bool mjthread_run_once(mjthread thread, mjThrdProc proc, void* arg) {
  if (!thread || thread->_type != MJTHREAD_NOTRUN || thread->_stop) return false;
  thread->_task.proc = proc;
  thread->_task.arg = arg;
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
  mjthread thread = arg;
  if (thread->_init.proc) thread->_init.proc(thread, thread->_init.arg);
  while (1) {
    // wait for routine and not shutdown
    pthread_mutex_lock(&thread->_lock);
    while (!thread->_task.proc && !thread->_stop) {
      pthread_cond_wait(&thread->_ready, &thread->_lock);
    }
    pthread_mutex_unlock(&thread->_lock);
    if (thread->_task.proc) {
      thread->_task.proc(thread, thread->_task.arg);
      // clean for next task
      thread->_task.proc = NULL;
      thread->_task.arg = NULL;
      if (thread->_callback.proc) {
        thread->_callback.proc(thread, thread->_callback.arg);
      }
    } else if (!thread->_stop) {
      MJLOG_ERR("thread wake up, not task or _stop!!!");
    }
    if (thread->_stop) break;
  }
  pthread_mutex_destroy(&thread->_lock);
  pthread_cond_destroy(&thread->_ready);
  thread->_type = MJTHREAD_NOTRUN;
  pthread_exit(NULL);
}

/*
===============================================================================
mjthread_set_task
  add RT to thread, return false if thread is working
===============================================================================
*/
bool mjthread_set_task(mjthread thread, mjThrdProc proc, void* arg) {
  if (!thread || thread->_type != MJTHREAD_NORMAL || !proc || thread->_stop) {
    return false;
  }
  // add worker to thread
  pthread_mutex_lock(&thread->_lock);
  if (thread->_task.proc) {
    pthread_mutex_unlock(&thread->_lock);
    return false;
  }
  thread->_task.proc = proc;
  thread->_task.arg = arg;
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
  if (!thread || thread->_type != MJTHREAD_NOTRUN) return false;
  // init fields
  pthread_mutex_init(&thread->_lock, NULL);
  pthread_cond_init(&thread->_ready, NULL);
  thread->_type = MJTHREAD_NORMAL;
  pthread_create(&thread->_id, NULL, mjthread_routine, thread);
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
    goto failout;
  }
  thread->_local = mjmap_new(31);
  if (!thread->_local) {
    MJLOG_ERR("mjmap_new error");
    goto failout;
  }
  return thread;

failout:
  free(thread);
  return NULL;
}

/*
===============================================================================
mjthread_delete
  stop thread, once routine thread never call this
===============================================================================
*/
bool mjthread_delete(mjthread thread) {
  if (!thread || thread->_stop || thread->_type == MJTHREAD_ONCE) return false;
  thread->_stop = true;
  if (thread->_type == MJTHREAD_NORMAL) {
    pthread_cond_broadcast(&thread->_ready);
    pthread_join(thread->_id, NULL);
  } 
  mjmap_delete(thread->_local);
  free(thread);
  return true;
}

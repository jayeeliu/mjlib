#include <stdlib.h>
#include <pthread.h>
#include "mjthread.h"
#include "mjlog.h"
#include "mjthreadpool.h"

/*
===============================================================================
mjthread_RunOnce
  create thread and run Routine
===============================================================================
*/
static void* mjthread_once_routine(void* arg) {
  // create and detach thread
  mjthread thread = (mjthread) arg;
  if (thread->_Init) thread->_Init(thread);
  if (thread->_Routine) thread->_Routine(thread);
  mjmap_delete(thread->_arg_map);
  free(thread);
  return NULL;
}

/*
===============================================================================
mjthread_new_once
  create thread run once and exit
===============================================================================
*/
bool mjthread_new_once(mjProc Init, void* init_arg, mjProc Routine, 
    void* arg) {
  // alloc mjthread struct
  mjthread thread = (mjthread) calloc(1, sizeof(struct mjthread));
  if (!thread) {
    MJLOG_ERR("mjthread create error");
    return false;
  }
  thread->_Init = Init;
  thread->init_arg = init_arg;
  thread->_Routine = Routine;
  thread->arg = arg;
  // init arg_map
  thread->_arg_map = mjmap_new(31);
  if (!thread->_arg_map) {
    MJLOG_ERR("mjmap_new error");
    free(thread);
    return false;
  }
  // init fields
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
  // arg can't be null
  mjthread thread = (mjthread) arg;
  // call init Routine
  if (thread->_Init) thread->_Init(thread);
  // threadloop 
  while (1) {
    // wait for routine and not shutdown
    pthread_mutex_lock(&thread->_thread_lock);
    while (!thread->_running && !thread->_shutdown) {
      pthread_cond_wait(&thread->_thread_ready, &thread->_thread_lock);
    }
    pthread_mutex_unlock(&thread->_thread_lock);
    // call routine
    if (thread->_Routine) thread->_Routine(thread);
    // should shutdown, break
    if (thread->_shutdown) break;
    // clean for next task
    thread->_Routine = NULL;
    thread->arg = NULL;
    thread->_running = false;
    // if in threadpool, add to freelist
    mjthreadentry entry = mjmap_get_obj(thread->_arg_map, "entry");
    if (entry) {
      pthread_mutex_lock(&entry->tpool->freelist_lock);
      list_add_tail(&entry->nodeList, &entry->tpool->freelist);
      pthread_mutex_unlock(&entry->tpool->freelist_lock);
    }
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
bool mjthread_add_routine(mjthread thread, mjProc Routine, void* arg) {
  // sanity check
  if (!thread) {
    MJLOG_ERR("thread is null");
    return false;
  }
  if (!Routine) return true;
  // add worker to thread
  pthread_mutex_lock(&thread->_thread_lock);
  bool retval = false;
  if (!thread->_running) {
    thread->_Routine = Routine;
    thread->arg = arg;
    thread->_running = true;
    pthread_cond_signal(&thread->_thread_ready);
    retval = true; 
  } else {
    mjthreadentry entry = mjmap_get_obj(thread->_arg_map, "entry");
    if (entry) {
      MJLOG_ERR("Oops: thread is busy, can't happen in threadpool");
    }
  }
  pthread_mutex_unlock(&thread->_thread_lock);
  return retval;
}

/*
===============================================================================
mjthread_set_obj
  set mjthread object
===============================================================================
*/
bool mjthread_set_obj(mjthread thread, const char* key, void* obj,
    mjProc obj_free) {
  if (!thread || !key) {
    MJLOG_ERR("thread or key is null");
    return false;
  }
  if (mjmap_set_obj(thread->_arg_map, key, obj, obj_free) < 0) return false;
  return true;
}

/*
===============================================================================
mjthread_get_obj
  get mjthread obj
===============================================================================
*/
void* mjthread_get_obj(mjthread thread, const char* key) {
  if (!thread || !key) {
    MJLOG_ERR("thread or key is null");
    return false;
  }
  return mjmap_get_obj(thread->_arg_map, key);
}

/*
===============================================================================
mjthread_new
  create new thread, run mjthread_normal_routine
===============================================================================
*/
mjthread mjthread_new(mjProc Init, void* init_arg) {
  // alloc mjthread struct
  mjthread thread = (mjthread) calloc(1, sizeof(struct mjthread));
  if (!thread) {
    MJLOG_ERR("mjthread create error");
    return NULL;
  }
  thread->_Init = Init;
  thread->init_arg = init_arg;
  thread->_arg_map = mjmap_new(31);
  if (!thread->_arg_map) {
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
  // sanity check
  if (!thread) {
    MJLOG_ERR("thread is null");
    return false;
  }
  // cant' re enter
  if (thread->_shutdown) return false;
  thread->_shutdown = true;
  // only normal thread need broadcast 
  pthread_cond_broadcast(&thread->_thread_ready);
  // wait thread exit
  pthread_join(thread->_thread_id, NULL);
  // only normal thread need destory
  mjmap_delete(thread->_arg_map);
  pthread_mutex_destroy(&thread->_thread_lock);
  pthread_cond_destroy(&thread->_thread_ready);
  free(thread);
  return true;
}

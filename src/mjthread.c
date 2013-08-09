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
  if (thread->Init_Routine) {
    thread->local = thread->Init_Routine(thread);
  }
  if (thread->Routine) {
    thread->Routine(thread);
  }
  if (thread->Exit_Routine) {
    thread->Exit_Routine(thread);
  }
  free(thread);
  return NULL;
}

/*
===============================================================================
mjthread_new_once
  create thread run once and exit
===============================================================================
*/
bool mjthread_new_once(mjProc Init_Routine, mjProc Exit_Routine, 
    void* local, mjProc Routine, void* arg) {
  // alloc mjthread struct
  mjthread thread = (mjthread) calloc(1, sizeof(struct mjthread));
  if (!thread) {
    MJLOG_ERR("mjthread create error");
    return false;
  }
  thread->Init_Routine = Init_Routine;
  thread->Exit_Routine = Exit_Routine;
  thread->local = local;
  thread->Routine = Routine;
  thread->arg = arg;
  // init fields
  pthread_create(&thread->thread_id, NULL, mjthread_once_routine, thread);
  pthread_detach(thread->thread_id);
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
  if (thread->Init_Routine) {
    thread->local = thread->Init_Routine(thread);
  }
  // threadloop 
  while (1) {
    // wait for routine and not shutdown
    pthread_mutex_lock(&thread->thread_lock);
    while (!thread->running && !thread->shutdown) {
      pthread_cond_wait(&thread->thread_ready, &thread->thread_lock);
    }
    pthread_mutex_unlock(&thread->thread_lock);
    // call routine
    if (thread->Routine) thread->Routine(thread);
    // should shutdown, break
    if (thread->shutdown) break;
    // clean for next task
    thread->Routine = NULL;
    thread->arg = NULL;
    thread->running = false;
    // if in threadpool, add to freelist
    if (thread->entry) {
      pthread_mutex_lock(&thread->entry->tpool->freelist_lock);
      list_add_tail(&thread->entry->nodeList, &thread->entry->tpool->freelist);
      pthread_mutex_unlock(&thread->entry->tpool->freelist_lock);
    }
  }
  // call exit Routine
  if (thread->Exit_Routine) {
    thread->Exit_Routine(thread);
  }
  thread->closed = true;
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
  pthread_mutex_lock(&thread->thread_lock);
  bool retval = false;
  if (!thread->running) {
    thread->Routine = Routine;
    thread->arg = arg;
    thread->running = true;
    pthread_cond_signal(&thread->thread_ready);
    retval = true; 
  } else if (thread->entry) {
    MJLOG_ERR("Oops: thread is busy, can't happen in threadpool");
  }
  pthread_mutex_unlock(&thread->thread_lock);
  return retval;
}

bool mjthread_set_local(mjthread thread, void* local) {
  if (!thread || thread->Init_Routine) {
    MJLOG_ERR("thread is null or has Init_Routine");
    return false;
  }
  thread->local = local;
  return true;
}

/*
===============================================================================
mjthread_new
  create new thread, run mjthread_normal_routine
===============================================================================
*/
mjthread mjthread_new(mjProc Init_Routine, mjProc Exit_Routine) {
  // alloc mjthread struct
  mjthread thread = (mjthread) calloc(1, sizeof(struct mjthread));
  if (!thread) {
    MJLOG_ERR("mjthread create error");
    return NULL;
  }
  thread->Init_Routine = Init_Routine;
  thread->Exit_Routine = Exit_Routine;
  // init fields
  pthread_mutex_init(&thread->thread_lock, NULL);
  pthread_cond_init(&thread->thread_ready, NULL);
  pthread_create(&thread->thread_id, NULL, mjthread_normal_routine, thread);
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
  if (thread->shutdown) return false;
  thread->shutdown = true;
  // only normal thread need broadcast 
  pthread_cond_broadcast(&thread->thread_ready);
  // wait thread exit
  pthread_join(thread->thread_id, NULL);
  // only normal thread need destory
  pthread_mutex_destroy(&thread->thread_lock);
  pthread_cond_destroy(&thread->thread_ready);
  free(thread);
  return true;
}

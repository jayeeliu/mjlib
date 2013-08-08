#include <stdlib.h>
#include <pthread.h>
#include "mjthread.h"
#include "mjlog.h"

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
    thread->local = thread->Init_Routine(thread->local);
  }
  if (thread->Routine) {
    thread->Routine(thread->local);
  }
  if (thread->Exit_Routine) {
    thread->Exit_Routine(thread->local);
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
    mjProc Routine, void* local) {
  // alloc mjthread struct
  mjthread thread = (mjthread) calloc(1, sizeof(struct mjthread));
  if (!thread) {
    MJLOG_ERR("mjthread create error");
    return false;
  }
  thread->Init_Routine = Init_Routine;
  thread->Routine = Routine;
  thread->Exit_Routine = Exit_Routine;
  thread->local = local;
  // init fields
  pthread_create(&thread->thread_id, NULL, mjthread_once_routine, thread);
  pthread_detach(thread->thread_id);
  return thread;
}

/*
===============================================================================
ThreadRoutine:
  used for short caculate task
===============================================================================
*/
static void* mjthread_normal_routine(void* arg) {
  // arg can't be null
  mjthread  thread = (mjthread) arg;
  mjProc    Routine;
  // call init Routine
  if (thread->Init_Routine) {
    thread->local = thread->Init_Routine(thread->local);
  }
  // threadloop 
  while (1) {
    // wait for routine and not shutdown
    pthread_mutex_lock(&thread->thread_lock);
    while (!thread->Routine && !thread->shutdown) {
      pthread_cond_wait(&thread->thread_ready, &thread->thread_lock);
    }
    // get worker value and clean old
    Routine = thread->Routine;
    thread->Routine = NULL;
    pthread_mutex_unlock(&thread->thread_lock);
    // should shutdown, break
    if (thread->shutdown) break;
    // call routine
    if (Routine) Routine(thread->local);
  }
  // call exit Routine
  if (thread->Exit_Routine) {
    thread->Exit_Routine(thread->local);
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
bool mjthread_add_routine(mjthread thread, mjProc Routine) {
  // sanity check
  if (!thread) {
    MJLOG_ERR("thread is null");
    return false;
  }
  if (!Routine) return true;
  // add worker to thread
  pthread_mutex_lock(&thread->thread_lock);
  bool retval = false;
  if (!thread->Routine) {
    thread->Routine = Routine;
    pthread_cond_signal(&thread->thread_ready);
    retval = true; 
  } else {
    MJLOG_ERR("Oops: thread is busy, can't happen in threadpool");
  }
  pthread_mutex_unlock(&thread->thread_lock);
  return retval;
}

/*
===============================================================================
mjthread_new
  create new thread, run mjthread_normal_routine
===============================================================================
*/
mjthread mjthread_new(mjProc Init_Routine, mjProc Exit_Routine, void* local) {
  // alloc mjthread struct
  mjthread thread = (mjthread) calloc(1, sizeof(struct mjthread));
  if (!thread) {
    MJLOG_ERR("mjthread create error");
    return NULL;
  }
  thread->Init_Routine = Init_Routine;
  thread->Exit_Routine = Exit_Routine;
  thread->local = local;
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
  if (thread->closed) {
    MJLOG_ERR("something wrong");
  }
  // only normal thread need destory
  pthread_mutex_destroy(&thread->thread_lock);
  pthread_cond_destroy(&thread->thread_ready);
  free(thread);
  return true;
}

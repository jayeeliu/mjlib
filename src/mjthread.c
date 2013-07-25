#include <stdlib.h>
#include <pthread.h>
#include "mjthread.h"
#include "mjlog.h"

/*
===============================================================================
mjThread_RunOnce
  create thread and run Routine
===============================================================================
*/
bool mjThread_RunOnce(mjProc Routine, void* arg) {
  // create and detach thread
  pthread_t tid;
  if (pthread_create(&tid, NULL, Routine, arg)) return false;
  pthread_detach(tid);
  return true;
}

/*
===============================================================================
ThreadRoutine:
  used for short caculate task
===============================================================================
*/
static void* Normal_routine(void* data) {
  // arg can't be null
  mjThread  thread = (mjThread) data;
  mjProc    PreRoutine, PostRoutine, Routine;
  void*     arg;
  void*     argPre;
  void*     argPost;
  // threadloop 
  while (1) {
    // wait for routine and not shutdown
    pthread_mutex_lock(&thread->thread_lock);
    while (!thread->Routine && !thread->shutdown) {
      pthread_cond_wait(&thread->thread_ready, &thread->thread_lock);
    }
    // get worker value
    PreRoutine  = thread->PreRoutine;
    argPre      = thread->argPre;
    PostRoutine = thread->PostRoutine;
    argPost     = thread->argPost;
    Routine     = thread->Routine;
    arg         = thread->arg;
    // clean worker value 
    thread->PreRoutine = thread->Routine = thread->PostRoutine = NULL;
    thread->argPre = thread->arg = thread->argPost = NULL;
    pthread_mutex_unlock(&thread->thread_lock);
    // should shutdown, break
    if (thread->shutdown) break;
    // call routine
    if (PreRoutine) PreRoutine(argPre);
    if (Routine) Routine(arg);
    if (PostRoutine) PostRoutine(argPost);
  }
  thread->closed = true;
  pthread_exit(NULL);
}

/*
===============================================================================
mjThread_AddWork
  add Routine to thread
===============================================================================
*/
bool mjThread_AddWork(mjThread thread, mjProc Routine, void* arg,
      mjProc PreRoutine, void* argPre, mjProc PostRoutine, void* argPost) {
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
    thread->Routine     = Routine;
    thread->arg         = arg;
    thread->PreRoutine  = PreRoutine;
    thread->argPre      = argPre;
    thread->PostRoutine = PostRoutine;
    thread->argPost     = argPost; 
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
mjthread_set_private
  set private data and freeprivate Proc
===============================================================================
*/
bool mjthread_set_private(mjThread thread, void* private, mjProc FreePrivate) {
  if (!thread) {
    MJLOG_ERR("thread is null");
    return false;
  }
  thread->private     = private;
  thread->FreePrivate = FreePrivate;
  return true;
}

/*
===============================================================================
mjthread_new
  create new thread, run Normal_routine
===============================================================================
*/
mjThread mjthread_new() {
  // alloc mjThread struct
  mjThread thread = (mjThread) calloc(1, sizeof(struct mjThread));
  if (!thread) {
    MJLOG_ERR("mjthread create error");
    return NULL;
  }
  // init fields 
  pthread_mutex_init(&thread->thread_lock, NULL);
  pthread_cond_init(&thread->thread_ready, NULL);
  pthread_create(&thread->thread_id, NULL, Normal_routine, thread);
  return thread;
}

/*
===============================================================================
mjthread_delete
  stop thread
===============================================================================
*/
bool mjthread_delete(mjThread thread) {
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
  // free private
  if (thread->FreePrivate && thread->private) {
    thread->FreePrivate(thread->private);
  }
  // only normal thread need destory
  pthread_mutex_destroy(&thread->thread_lock);
  pthread_cond_destroy(&thread->thread_ready);
  free(thread);
  return true;
}

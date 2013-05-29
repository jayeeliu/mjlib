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
static void* NormalRoutine(void* data) {
  // arg can't be null
  mjThread  thread = (mjThread) data;
  mjProc    PreRoutine, PostRoutine, Routine;
  void*     arg;
  void*     argPre;
  void*     argPost;
  // threadloop 
  while (1) {
    // wait for routine and not shutdown
    pthread_mutex_lock(&thread->threadLock);
    while (!thread->Routine && !thread->shutDown) {
      pthread_cond_wait(&thread->threadReady, &thread->threadLock);
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
    pthread_mutex_unlock(&thread->threadLock);
    // should shutdown, break
    if (thread->shutDown) break;
    // call routine
    if (PreRoutine) PreRoutine(argPre);
    if (Routine) Routine(arg);
    if (PostRoutine) PostRoutine(argPost);
  }
  thread->closed = 1;
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
  pthread_mutex_lock(&thread->threadLock);
  bool retval = false;
  if (!thread->Routine) {
    thread->Routine     = Routine;
    thread->arg         = arg;
    thread->PreRoutine  = PreRoutine;
    thread->argPre      = argPre;
    thread->PostRoutine = PostRoutine;
    thread->argPost     = argPost; 
    pthread_cond_signal(&thread->threadReady);
    retval = true; 
  } else {
    MJLOG_ERR("Oops: thread is busy, can't happen in threadpool");
  }
  pthread_mutex_unlock(&thread->threadLock);
  return retval;
}

/*
===============================================================================
mjThread_SetPrivate
  set private data and freeprivate Proc
===============================================================================
*/
bool mjThread_SetPrivate(mjThread thread, void* private, mjProc FreePrivate) {
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
mjThread_New
  create new thread, run NormalRoutine
===============================================================================
*/
mjThread mjThread_New() {
  // alloc mjThread struct
  mjThread thread = (mjThread) calloc(1, sizeof(struct mjThread));
  if (!thread) {
    MJLOG_ERR("mjthread create error");
    return NULL;
  }
  // init fields 
  pthread_mutex_init(&thread->threadLock, NULL);
  pthread_cond_init(&thread->threadReady, NULL);
  pthread_create(&thread->threadID, NULL, NormalRoutine, thread);
  return thread;
}

/*
===============================================================================
mjThread_Delete
  stop thread
===============================================================================
*/
bool mjThread_Delete(mjThread thread) {
  // sanity check
  if (!thread) {
    MJLOG_ERR("thread is null");
    return false;
  }
  // cant' re enter
  if (thread->shutDown) return false;
  thread->shutDown = 1;
  // only normal thread need broadcast 
  pthread_cond_broadcast(&thread->threadReady);
  // wait thread exit
  pthread_join(thread->threadID, NULL);
  if (thread->closed != 1) {
    MJLOG_ERR("something wrong");
  }
  // free private
  if (thread->FreePrivate && thread->private) {
    thread->FreePrivate(thread->private);
  }
  // only normal thread need destory
  pthread_mutex_destroy(&thread->threadLock);
  pthread_cond_destroy(&thread->threadReady);
  free(thread);
  return true;
}

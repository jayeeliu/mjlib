#ifndef _MJTHREAD_H
#define _MJTHREAD_H

#include <stdbool.h>
#include <pthread.h>
#include "mjproc.h"

struct mjthreadentry;

struct mjthread {
  pthread_t             thread_id;
  pthread_mutex_t       thread_lock;
  pthread_cond_t        thread_ready;
  struct mjthreadentry* entry;    // entry when in threadpool, otherwise NULL

  mjProc  Init_Routine;   // run once when thread init
  mjProc  Exit_Routine;   // run once when thread exit
  void*   local;          // thread local data 

  mjProc  Routine;        // routine to be run
  void*   arg;

  bool    running;
  bool    closed;         // 1 when thread exit, otherwise 0
  bool    shutdown;       // 1 when shutdown command has invoked, otherwise 0
};
typedef struct mjthread* mjthread;

extern bool     mjthread_new_once(mjProc Init_Routine, mjProc Exit_Routine, 
    void* local, mjProc Routine, void* arg);

extern bool     mjthread_add_routine(mjthread thread, mjProc Routine, void* arg);
extern bool     mjthread_set_local(mjthread thread, void* local);
extern mjthread mjthread_new(mjProc Init_Routine, mjProc Exit_Routine);
extern bool     mjthread_delete(mjthread thread);

#endif

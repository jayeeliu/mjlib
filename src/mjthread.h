#ifndef _MJTHREAD_H
#define _MJTHREAD_H

#include <stdbool.h>
#include <pthread.h>
#include "mjproc.h"
#include "mjmap.h"

//struct mjthreadentry;

struct mjthread {
  pthread_t             thread_id;
  pthread_mutex_t       thread_lock;
  pthread_cond_t        thread_ready;

  mjProc  Init_Thread;                // run once when thread init
  void*   init_arg;
  mjProc  Exit_Thread;                // run once when thread exit
  mjProc  Routine;                    // routine to be run
  void*   arg;

  mjmap   arg_map;                    // arg map for this thread

  bool    running;
  bool    closed;         // 1 when thread exit, otherwise 0
  bool    shutdown;       // 1 when shutdown command has invoked, otherwise 0
};
typedef struct mjthread* mjthread;

extern bool     mjthread_new_once(mjProc Init_Thread, void* init_arg, 
    mjProc Exit_Thread, void* thread_local, mjProc Routine, void* arg);

extern bool     mjthread_add_routine(mjthread thread, mjProc Routine, void* arg);
extern bool     mjthread_set_local(mjthread thread, void* thread_local);
extern mjthread mjthread_new(mjProc Init_Thread, void* init_arg, mjProc Exit_Thread);
extern bool     mjthread_delete(mjthread thread);

#endif

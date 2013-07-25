#ifndef _MJTHREAD_H
#define _MJTHREAD_H

#include <stdbool.h>
#include <pthread.h>
#include "mjproc.h"

struct mjThread {
  pthread_t       thread_id;
  pthread_mutex_t thread_lock;
  pthread_cond_t  thread_ready;
    
  mjProc          Routine;
  void*           arg; 
  mjProc          PreRoutine;
  void*           argPre;
  mjProc          PostRoutine;
  void*           argPost;
    
  void*           private;        // holding private data, point to threadpool when in threadpool
  mjProc          FreePrivate;

  bool            closed;         // 1 when thread exit, otherwise 0
  bool            shutdown;       // 1 when shutdown command has invoked, otherwise 0
};
typedef struct mjThread* mjThread;

extern bool     mjThread_RunOnce(mjProc Routine, void* arg);
extern bool     mjThread_AddWork(mjThread thread, mjProc Routine, void* arg,
                    mjProc PreRoutine, void* argPre, mjProc PostRoutine, void* argPost);
extern bool     mjthread_set_private(mjThread thread, void* private, mjProc FreePrivate);

extern mjThread mjthread_new();
extern bool     mjthread_delete(mjThread thread);

#endif

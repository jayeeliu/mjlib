#ifndef _MJTHREADPOOL_H
#define _MJTHREADPOOL_H

#include <stdbool.h>
#include <pthread.h>
#include "mjlist.h"
#include "mjthread.h"
#include "mjlockless.h"

// threadpool struct
struct mjthreadpool {
  bool        _shutdown;         // shutdown this thread pool?
  int         _max_thread;
  mjProc      _Init;
  void*       init_arg;
  mjlockless  _free_list;
  mjthread    _threads[0];
};
typedef struct mjthreadpool* mjthreadpool;

extern bool         mjthreadpool_add_routine(mjthreadpool tpool, mjProc Routine, void* arg);
extern bool         mjthreadpool_add_routine_plus(mjthreadpool tpool, mjProc Routine, void* arg);
extern mjthreadpool mjthreadpool_new(int max_thread, mjProc Init_Routine, void* init_arg);
extern bool         mjthreadpool_delete(mjthreadpool tpool);

#endif

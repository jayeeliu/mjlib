#ifndef _MJTHREADPOOL_H
#define _MJTHREADPOOL_H

#include <stdbool.h>
#include <pthread.h>
#include "mjlist.h"
#include "mjthread.h"

struct mjthreadpool;

struct mjthreadentry {
  struct mjthreadpool*  tpool;
  struct list_head      nodeList;
  mjthread              thread;
  mjProc                Routine;
  void*                 arg;
};
typedef struct mjthreadentry* mjthreadentry;

// threadpool struct
struct mjthreadpool {
  pthread_mutex_t       freelist_lock;    // lock for threadList  
  struct list_head      freelist;         // task list 
  bool                  shutdown;         // shutdown this thread pool?
  int                   max_thread;
  mjProc                Thread_Init_Routine;
  mjProc                Thread_Exit_Routine;
  struct mjthreadentry  thread_entrys[0];
};
typedef struct mjthreadpool*  mjthreadpool;

extern bool         mjthreadpool_add_routine(mjthreadpool tpool, mjProc Routine, void* arg);
extern bool         mjthreadpool_add_routine_plus(mjthreadpool tpool, mjProc Routine, void* arg);
extern mjthreadpool mjthreadpool_new(int max_thread, mjProc Init_Routine, mjProc Exit_Routine);
extern bool         mjthreadpool_delete(mjthreadpool tpool);

#endif

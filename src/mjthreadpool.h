#ifndef _MJTHREADPOOL_H
#define _MJTHREADPOOL_H

#include <stdbool.h>
#include <pthread.h>
#include "mjlist.h"
#include "mjthread.h"

struct mjThreadPool;

struct mjThreadEntry {
  struct mjThreadPool*  tPool;
  struct list_head      nodeList;
  mjthread              thread;
  mjProc                Init_Routine;
  mjProc                Exit_Routine;
  mjProc                Routine;
  void*                 arg;
};
typedef struct mjThreadEntry* mjThreadEntry;

// threadpool struct
struct mjThreadPool {
  pthread_mutex_t       freelist_lock;   // lock for threadList  
  struct list_head      freelist;       // task list 
  int                   shutdown;       // shutdown this thread pool?
  int                   max_thread;
  mjProc                Thread_Init_Proc;
  struct mjThreadEntry  threads_entry[0];
};
typedef struct mjThreadPool*  mjThreadPool;

extern bool         mjThreadPool_AddWork(mjThreadPool tPool, mjProc Routine, void* arg);
extern bool         mjThreadPool_AddWorkPlus(mjThreadPool tPool, mjProc Routine, void* arg);
extern mjThreadPool mjThreadPool_New(int max_thread);
extern mjThreadPool mjthreadpool_new(int max_thread, mjProc Init_Proc);
extern bool         mjThreadPool_Delete(mjThreadPool tPool);

#endif

#ifndef _MJTHREADPOOL_H
#define _MJTHREADPOOL_H

#include <stdbool.h>
#include <pthread.h>
#include "mjlist.h"
#include "mjthread.h"

// threadpool struct
struct mjThreadPool2 {
    pthread_mutex_t         threadListLock; // lock for threadList
	struct list_head        threadList;     // task list 
	int                     shutDown;       // shutdown this thread pool?
    int                     threadNum;      // max thread in thread pool  
};
typedef struct mjThreadPool2*  mjThreadPool2; 

extern bool         mjThreadPool2_AddWorker( mjThreadPool2 tpool, mjProc thread, void* arg );
extern bool         mjThreadPool2_AddThread( mjThreadPool2 tpool, mjThread thread );
extern mjThreadPool mjThreadPool2_New();
extern bool         mjThreadPool2_Delete( mjThreadPool2 tPool );

#endif

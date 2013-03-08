#ifndef _MJTHREADPOOL_H
#define _MJTHREADPOOL_H

#include <stdbool.h>
#include <pthread.h>
#include "mjlist.h"

typedef void* mjthread( void* arg );

struct mjThreadPool;

struct mjThread {
    struct mjThreadPool*    threadPool;             // the threadPool
    struct list_head        nodeList;               // node in list, entry for threadList
    pthread_t               threadID;               // threadID

    pthread_mutex_t         threadLock;             // threadLock for threadWorker
    pthread_cond_t          threadReady;            // threadReady condition
    mjthread*               ThreadWorker;
    void*                   threadArg;
};
typedef struct mjThread* mjThread;

// threadpool struct
struct mjThreadPool {
    pthread_mutex_t     threadListLock;             // lock for threadList
	struct list_head    threadList;                 // task list 
	int                 shutDown;                   // shutdown this thread pool?
    int                 maxThreadNum;               // max thread in thread pool  
    struct mjThread     threads[0];                 // array of thread id
};
typedef struct mjThreadPool*  mjThreadPool; 

extern bool         mjThreadPool_AddWorker( mjThreadPool tpool, mjthread* thread, void* arg );
extern mjThreadPool mjThreadPool_New( int maxThreadNum );
extern bool         mjThreadPool_Delete( mjThreadPool tPool );

#endif

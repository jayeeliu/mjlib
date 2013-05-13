#ifndef _MJTHREADPOOL2_H
#define _MJTHREADPOOL2_H

#include <stdbool.h>
#include <pthread.h>
#include "mjlist.h"
#include "mjthread.h"

struct mjThreadPool2;

struct mjThreadEntry2 {
    struct mjThreadPool2*   tPool;
    struct list_head        nodeList;
    mjThread                thread;
};
typedef struct mjThreadEntry2* mjThreadEntry2;

// threadpool struct
struct mjThreadPool2 {
    pthread_mutex_t         freeListLock;   // lock for threadList
    struct list_head        freeList;       // task list 
    int                     shutDown;       // shutdown this thread pool?
    int                     maxThread;
    struct mjThreadEntry2   threads[0];
};
typedef struct mjThreadPool2*  mjThreadPool2;

extern bool             mjThreadPool2_AddWork( mjThreadPool2 tPool, mjProc Routine, void* arg );
extern mjThreadPool2    mjThreadPool2_New( int maxThread );
extern bool             mjThreadPool2_Delete( mjThreadPool2 tPool );

#endif

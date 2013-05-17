#ifndef _MJTHREADPOOL2_H
#define _MJTHREADPOOL2_H

#include <stdbool.h>
#include <pthread.h>
#include "mjlist.h"
#include "mjthread.h"

struct mjThreadPool;

struct mjThreadEntry {
    struct mjThreadPool*    tPool;
    struct list_head        nodeList;
    mjThread                thread;
};
typedef struct mjThreadEntry* mjThreadEntry;

// threadpool struct
struct mjThreadPool {
    pthread_mutex_t         freeListLock;   // lock for threadList
    struct list_head        freeList;       // task list 
    int                     shutDown;       // shutdown this thread pool?
    int                     maxThread;
    struct mjThreadEntry    threads[0];
};
typedef struct mjThreadPool*  mjThreadPool;

extern bool             mjThreadPool_AddWork( mjThreadPool tPool, 
                                    mjProc Routine, void* arg );
extern bool             mjThreadPool_AddWorkPlus( mjThreadPool tPool, 
                                    mjProc Routine, void* arg );
extern mjThreadPool     mjThreadPool_New( int maxThread );
extern bool             mjThreadPool_Delete( mjThreadPool tPool );

#endif

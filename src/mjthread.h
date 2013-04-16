#ifndef __MJTHREAD_H
#define __MJTHREAD_H

#include <stdbool.h>
#include <pthread.h>

typedef void* mjthread( void* arg );

#define MJTHREAD_FREE   0
#define MJTHREAD_READY  1
#define MJTHREAD_BUSY   2

struct mjThread {
    pthread_t       threadID;
    pthread_mutex_t threadLock;
    pthread_cond_t  threadReady;
    
    mjthread*       ThreadWorker;
    void*           threadArg; 

    int             shutDown;
    int             status;
};
typedef struct mjThread* mjThread;

extern bool     mjThread_RunOnce( mjthread* routine, void* arg );

extern bool     mjThread_AddWork( mjThread thread, mjthread* ThreadWorker, void* arg );
extern mjThread mjThread_New();
extern mjThread mjThread_NewLoop( mjthread* ThreadWorker, void* threadArg );
extern bool     mjThread_Delete( mjThread thread );

#endif

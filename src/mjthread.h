#ifndef __MJTHREAD_H
#define __MJTHREAD_H

#include <stdbool.h>
#include <pthread.h>
#include "mjproc.h"

#define MJTHREAD_FREE   0
#define MJTHREAD_READY  1
#define MJTHREAD_BUSY   2

struct mjThread {
    pthread_t       threadID;
    pthread_mutex_t threadLock;
    pthread_cond_t  threadReady;
    
    mjProc          Routine;
    void*           arg; 

    mjProc          PreRoutine;
    mjProc          PostRoutine;
    mjProc          FreePrivate;
    void*           private;        // holding private data, point to threadpool when in threadpool

    int             closed;
    int             shutDown;
};
typedef struct mjThread* mjThread;

extern bool     mjThread_RunOnce( mjProc Routine, void* arg );

extern bool     mjThread_AddWork( mjThread thread, mjProc Routine, void* arg );
extern bool     mjThread_SetPrivate( mjThread thread, void* private, mjProc FreePrivate );
extern bool     mjThread_SetPrePost( mjThread thread, mjProc PreRoutine, mjProc PostRoutine );
extern mjThread mjThread_New();
extern mjThread mjThread_NewLoop( mjProc ThreadWorker, void* threadArg );
extern bool     mjThread_Delete( mjThread thread );

#endif

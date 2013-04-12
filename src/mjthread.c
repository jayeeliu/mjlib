#include <stdlib.h>
#include <pthread.h>
#include "mjthread.h"
#include "mjlog.h"

bool mjThread_RunOnce( mjthread* routine, void* arg )
{
    pthread_t tid;
    pthread_create( &tid, NULL, routine, arg );
    pthread_detach( tid );
    return true;
}

static void* ThreadRoutine( void* arg )
{
    if ( !arg ) {
        MJLOG_ERR( "ThreadRoutine arg is null" );
        pthread_exit( NULL );
    }

    mjThread thread = ( mjThread ) arg;
    mjthread* worker;
    void*   workerArg;

    while ( 1 ) {
        pthread_mutex_lock( &thread->threadLock );
        while ( !thread->ThreadWorker && !thread->shutDown ) {
            pthread_cond_wait( &thread->threadReady, &thread->threadLock );
        }
        worker                  = thread->ThreadWorker;
        workerArg               = thread->threadArg;
        thread->ThreadWorker    = NULL;
        thread->threadArg       = NULL;
        thread->status          = MJTHREAD_BUSY;
        pthread_mutex_unlock( &thread->threadLock );

        if ( worker ) {
            ( *worker ) ( workerArg );
        }
        if ( thread->shutDown ) break;
        
        pthread_mutex_lock( &thread->threadLock );
        thread->status = MJTHREAD_FREE;
        pthread_mutex_unlock( &thread->threadLock );
    }
    pthread_exit( NULL );
}

bool mjThread_AddWork( mjThread thread, mjthread* ThreadWorker, void* arg )
{
    if ( !thread ) return false;
    
    bool retval = false;

    pthread_mutex_lock( &thread->threadLock );
    if ( thread->status == MJTHREAD_FREE ) {
        thread->ThreadWorker    = ThreadWorker;
        thread->threadArg       = arg;
        thread->status = MJTHREAD_READY;
        pthread_cond_signal( &thread->threadReady );
        retval = true; 
    }
    pthread_mutex_unlock( &thread->threadLock );

    return retval;
}

mjThread mjThread_New()
{
    mjThread thread = ( mjThread ) calloc(1, sizeof( struct mjThread ) );
    if ( !thread ) {
        MJLOG_ERR( "mjthread create error" );
        return NULL;
    }
   
    thread->shutDown        = 0; 
    thread->status          = MJTHREAD_FREE;
    pthread_mutex_init( &thread->threadLock, NULL );
    pthread_cond_init( &thread->threadReady, NULL );
    thread->ThreadWorker    = NULL;
    thread->threadArg       = NULL;
    pthread_create( &thread->threadID, NULL, ThreadRoutine, thread );

    return thread;
}

bool mjThread_Delete( mjThread thread )
{
    if ( !thread ) {
        MJLOG_ERR( "thread is null" );
        return false;
    }
    if ( thread->shutDown ) return false;
    
    thread->shutDown = 1;
    pthread_cond_broadcast( &thread->threadReady );
    pthread_join( thread->threadID, NULL );
    free( thread );
    return true;
}

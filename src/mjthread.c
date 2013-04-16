#include <stdlib.h>
#include <pthread.h>
#include "mjthread.h"
#include "mjlog.h"

bool mjThread_RunOnce( mjProc Routine, void* arg )
{
    pthread_t tid;
    int ret = pthread_create( &tid, NULL, Routine, arg );
    if ( ret ) return false;

    pthread_detach( tid );
    return true;
}

/*
======================================================
ThradRoutine:
    used for short caculate task
======================================================
*/
static void* DefaultRoutine( void* arg )
{
    if ( !arg ) {
        MJLOG_ERR( "ThreadRoutine arg is null" );
        pthread_exit( NULL );
        return NULL;
    }

    mjThread thread = ( mjThread ) arg;

    while ( !thread->shutDown ) {
        pthread_mutex_lock( &thread->threadLock );
        while ( !thread->Routine && !thread->shutDown ) {
            pthread_cond_wait( &thread->threadReady, &thread->threadLock );
        }
        pthread_mutex_unlock( &thread->threadLock );

        if ( thread->Routine ) {
            ( *thread->Routine ) ( thread->arg );
        }
        
        pthread_mutex_lock( &thread->threadLock );
        thread->Routine = NULL;
        thread->arg     = NULL;
        pthread_mutex_unlock( &thread->threadLock );
    }
    thread->closed = 1;
    pthread_exit( NULL );
}

static void* LoopThreadRoutine( void* arg )
{
    if ( !arg ) {
        MJLOG_ERR( "LoopThreadRoutine arg is null" );
        pthread_exit( NULL );
        return NULL; 
    }

    mjThread thread = ( mjThread ) arg;
    while ( !thread->shutDown ) {
        ( *thread->Routine ) ( thread->arg );
    }

    thread->closed = 1;
    pthread_exit( NULL );
    return NULL;
}

bool mjThread_AddWork( mjThread thread, mjProc Routine, void* arg )
{
    if ( !thread ) {
        MJLOG_ERR( "thread is null" );
        return false;
    }
    
    bool retval = false;

    pthread_mutex_lock( &thread->threadLock );
    if ( !thread->Routine ) {
        thread->Routine = Routine;
        thread->arg     = arg;
        pthread_cond_signal( &thread->threadReady );
        retval = true; 
    }
    pthread_mutex_unlock( &thread->threadLock );

    return retval;
}

/*
=========================================================================
mjThread_New
    create new thread, run DefaultRoutine
=========================================================================
*/
mjThread mjThread_New()
{
    mjThread thread = ( mjThread ) calloc( 1, sizeof( struct mjThread ) );
    if ( !thread ) {
        MJLOG_ERR( "mjthread create error" );
        return NULL;
    }
   
    thread->shutDown    = 0; 
    thread->closed      = 0;
    thread->Routine     = NULL;
    thread->arg         = NULL;
    pthread_mutex_init( &thread->threadLock, NULL );
    pthread_cond_init( &thread->threadReady, NULL );
    pthread_create( &thread->threadID, NULL, DefaultRoutine, thread );

    return thread;
}

mjThread mjThread_NewLoop( mjProc Routine, void* arg )
{
    mjThread thread = ( mjThread ) calloc( 1, sizeof( struct mjThread ) );
    if ( !thread ) {
        MJLOG_ERR( "mjthread create error" );
        return NULL;
    }
    
    thread->shutDown    = 0;
    thread->closed      = 0;
    thread->Routine     = Routine;
    thread->arg         = arg;
    pthread_mutex_init( &thread->threadLock, NULL );
    pthread_cond_init( &thread->threadReady, NULL );
    pthread_create( &thread->threadID, NULL, LoopThreadRoutine, thread );

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
    if ( thread->closed != 1 ) {
        MJLOG_ERR( "something wrong" );
    }
    free( thread );
    return true;
}

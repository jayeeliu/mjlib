#include <stdlib.h>
#include <pthread.h>
#include "mjthread.h"
#include "mjlog.h"

/*
=========================================================
mjThread_RunOnce
    create thread and run Routine
=========================================================
*/
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
ThreadRoutine:
    used for short caculate task
======================================================
*/
static void* DefaultRoutine( void* arg )
{
    // arg can't be null
    mjThread    thread = ( mjThread ) arg;
    mjProc      PreRoutine;
    mjProc      PostRoutine;
    mjProc      Routine;
    void*       threadArg;
    
    while ( 1 ) {
        pthread_mutex_lock( &thread->threadLock );
        while ( !thread->Routine && !thread->shutDown ) {
            pthread_cond_wait( &thread->threadReady, &thread->threadLock );
        }
        PreRoutine  = thread->PreRoutine;
        PostRoutine = thread->PostRoutine;
        Routine     = thread->Routine;
        threadArg   = thread->arg;
        thread->Routine = thread->arg = NULL;
        pthread_mutex_unlock( &thread->threadLock );
        // should shutdown, break
        if ( thread->shutDown ) break;
        // call routine
        if ( PreRoutine ) ( *PreRoutine ) ( thread );
        if ( Routine ) ( *Routine ) ( threadArg );
        if ( PostRoutine ) ( *PostRoutine ) ( thread );
    }
    thread->closed = 1;
    pthread_exit( NULL );
}

/*
===================================================
DefaultLoopRoutine
    Default Loop Thread Routine
===================================================
*/
static void* DefaultLoopRoutine( void* arg )
{
    // arg can't be null
    mjThread thread = ( mjThread ) arg;

    while ( !thread->shutDown ) {
        ( *thread->Routine ) ( thread->arg );
    }
    thread->closed = 1;
    pthread_exit( NULL );
}

/*
====================================================================
mjThread_AddWork
    add Routine to thread
====================================================================
*/
bool mjThread_AddWork( mjThread thread, mjProc Routine, void* arg )
{
    if ( !thread ) {
        MJLOG_ERR( "thread is null" );
        return false;
    }
    if ( thread->type != MJTHREAD_NORMAL ) {
        MJLOG_ERR( "only normal thread can add work" );
        return false;
    }
    if ( !Routine ) return true;
    // add worker to thread
    pthread_mutex_lock( &thread->threadLock );
    bool retval = false;
    if ( !thread->Routine ) {
        thread->Routine = Routine;
        thread->arg     = arg;
        pthread_cond_signal( &thread->threadReady );
        retval = true; 
    } else {
        MJLOG_ERR( "thread is busy" );
    }
    pthread_mutex_unlock( &thread->threadLock );

    return retval;
}

/*
============================================================================
mjThread_SetPrivate
    set private data and freeprivate Proc
============================================================================
*/
bool mjThread_SetPrivate( mjThread thread, void* private, mjProc FreePrivate )
{
    if ( !thread ) {
        MJLOG_ERR( "thread is null" );
        return false;
    }
    thread->private     = private;
    thread->FreePrivate = FreePrivate;
    return true;
}

/*
=================================================================================
mjThread_SetPrePost
    set Pre and Post Routine
=================================================================================
*/
bool mjThread_SetPrePost( mjThread thread, mjProc PreRoutine, mjProc PostRoutine )
{
    if ( !thread ) {
        MJLOG_ERR( "thread is null" );
        return false;
    }
    thread->PreRoutine  = PreRoutine;
    thread->PostRoutine = PostRoutine;
    return true;
}

/*
=========================================================================
mjThread_New
    create new thread, run DefaultRoutine
=========================================================================
*/
mjThread mjThread_New()
{
    // alloc mjThread struct
    mjThread thread = ( mjThread ) calloc ( 1, sizeof( struct mjThread ) );
    if ( !thread ) {
        MJLOG_ERR( "mjthread create error" );
        return NULL;
    }
  
    thread->type = MJTHREAD_NORMAL; 
    pthread_mutex_init( &thread->threadLock, NULL );
    pthread_cond_init( &thread->threadReady, NULL );
    pthread_create( &thread->threadID, NULL, DefaultRoutine, thread );
    return thread;
}

/*
=====================================================================
mjThread_NewLoop
    create Loop Thread
=====================================================================
*/
mjThread mjThread_NewLoop( mjProc Routine, void* arg )
{
    if ( !Routine ) {
        MJLOG_ERR( "Loop Rountine can't be null");
        return NULL;
    }

    mjThread thread = ( mjThread ) calloc( 1, sizeof( struct mjThread ) );
    if ( !thread ) {
        MJLOG_ERR( "mjthread create error" );
        return NULL;
    }
   
    thread->type    = MJTHREAD_LOOP;
    thread->Routine = Routine;
    thread->arg     = arg;
    pthread_create( &thread->threadID, NULL, DefaultLoopRoutine, thread );
    return thread;
}

/*
=================================================
mjThread_Delete
    stop thread
=================================================
*/
bool mjThread_Delete( mjThread thread )
{
    if ( !thread ) {
        MJLOG_ERR( "thread is null" );
        return false;
    }
    // cant' re enter
    if ( thread->shutDown ) return false;
    thread->shutDown = 1;
    // only normal thread need broadcast 
    if ( thread->type == MJTHREAD_NORMAL ) {
        pthread_cond_broadcast( &thread->threadReady );
    }
    // wait thread exit
    pthread_join( thread->threadID, NULL );
    if ( thread->closed != 1 ) {
        MJLOG_ERR( "something wrong" );
    }
    // free private
    if ( thread->FreePrivate && thread->private ) {
        thread->FreePrivate( thread->private );
    }
    // only normal thread need destory
    if ( thread->type == MJTHREAD_NORMAL ) {
        pthread_mutex_destroy( &thread->threadLock );
        pthread_cond_destroy( &thread->threadReady );
    }
    free( thread );
    return true;
}

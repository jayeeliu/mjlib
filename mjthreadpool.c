#include <stdlib.h>

#include "mjlog.h"
#include "mjthreadpool.h"

/*
=======================================================================
ThreadRoutine
    threadpool main routine
=======================================================================
*/
static void* ThreadRoutine( void* arg ) 
{
    if ( !arg ) {
        MJLOG_ERR( "ThreadRoutine arg is null" );
        pthread_exit( NULL );
    }
    mjThread thread = ( mjThread )arg;          // get thread
    mjThreadPool tPool = thread->threadPool;    // get threadPool
    mjthread* worker;
    void* workerArg;

	while ( 1 ) { 
		// wait for cond
        pthread_mutex_lock( &thread->threadLock ); 
		while ( !thread->ThreadWorker && !tPool->shutDown ) { 
			pthread_cond_wait( &thread->threadReady, &thread->threadLock ); 
		} 
        // get worker
        worker      = thread->ThreadWorker;
        workerArg   = thread->threadArg;
        thread->ThreadWorker    = NULL;
        thread->threadArg       = NULL;
        pthread_mutex_unlock( &thread->threadLock );

		// call worker process 
		if ( worker ) {
		    ( *worker ) ( workerArg );
        }
		// destory the threadpool 
		if ( tPool->shutDown ) break; 

        // add thread to freelist
        pthread_mutex_lock( &tPool->threadListLock );
        list_add_tail( &thread->nodeList, &tPool->threadList );
        pthread_mutex_unlock( &tPool->threadListLock );
	}
	// thread shutdown
	pthread_exit( NULL ); 
} 

/*
===========================================================================
mjThreadPool_AddWorker
    add worker to thread pool
    return: 0 --- success, -1 --- fail
===========================================================================
*/ 
bool mjThreadPool_AddWorker( mjThreadPool tPool, mjthread* ThreadWorker, void* arg ) 
{ 
    if ( !tPool ) {
        MJLOG_ERR( "mjthread pool is null" );
        return false;
    }
    if ( tPool->shutDown ) {
        MJLOG_WARNING( "mjthreadpool is shutdown" );
        return false;
    }
    // no free thread
    if ( list_empty( &tPool->threadList ) ) return false;

    mjThread thread;
    // get free thread
	pthread_mutex_lock( &tPool->threadListLock ); 
    thread = list_first_entry( &tPool->threadList, 
                    struct mjThread, nodeList );
    if ( thread ) {
        list_del_init( &thread->nodeList );
    }
	pthread_mutex_unlock( &tPool->threadListLock ); 

	// wakeup one thread to run
	if ( thread ) {
        thread->ThreadWorker    = ThreadWorker;
        thread->threadArg       = arg;
	    pthread_cond_signal( &thread->threadReady ); 
        return true;
    }
	
    return false; 
}

/*
==========================================================
mjThreadPool_New
    init new thread pool
    return: NOT NULL--- mjThreadPool struct, NULL --- fail
==========================================================
*/
mjThreadPool mjThreadPool_New( int maxThreadNum ) 
{
    mjThreadPool tPool = ( mjThreadPool ) calloc( 1, 
            sizeof( struct mjThreadPool ) + maxThreadNum * sizeof( struct mjThread ) );
    if ( !tPool ) {
        MJLOG_ERR( "mjThreadPool alloc error" );
        return NULL;
    }

    pthread_mutex_init( &tPool->threadListLock, NULL ); 
	INIT_LIST_HEAD( &tPool->threadList ); 
	tPool->shutDown         = 0; 
	tPool->maxThreadNum     = maxThreadNum; 
    // create thread
	for ( int i = 0; i < maxThreadNum; i++ ) { 
        tPool->threads[i].threadPool = tPool;
        INIT_LIST_HEAD( &tPool->threads[i].nodeList );
		list_add_tail( &tPool->threads[i].nodeList, &tPool->threadList );
        // init thread worker
        pthread_mutex_init( &tPool->threads[i].threadLock, NULL );
        pthread_cond_init( &tPool->threads[i].threadReady, NULL );
        tPool->threads[i].ThreadWorker  = NULL;
        tPool->threads[i].threadArg     = NULL;
        pthread_create( &tPool->threads[i].threadID, NULL, ThreadRoutine, &tPool->threads[i] );
	}
    return tPool; 
} 

/* 
===========================================
mjThreadPool_delete
    destory thread pool
===========================================
*/
bool mjThreadPool_Delete( mjThreadPool tPool ) 
{ 
    if ( !tPool ) {
        MJLOG_ERR( "tPool is null" );
        return false;
    }
    // can't call it twice
	if ( tPool->shutDown ) return false;
	tPool->shutDown = 1; 
    // notify all thread
	for ( int i = 0; i < tPool->maxThreadNum; i++ ) {
	    pthread_cond_broadcast( &tPool->threads[i].threadReady ); 
    }
	// wait all thread to exit 
	for ( int i = 0; i < tPool->maxThreadNum; i++ ) { 
		pthread_join( tPool->threads[i].threadID, NULL ); 
    }

    free(tPool);
	return true; 
} 

#include <stdlib.h>

#include "mjlog.h"
#include "mjthreadpool2.h"

/*
===========================================================================
mjThreadPool_AddWorker
    add worker to thread pool
    return: 0 --- success, -1 --- fail
===========================================================================
*/ 
bool mjThreadPool2_AddWorker( mjThreadPool2 tPool, mjProc ThreadWorker, void* arg ) 
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

    mjThreadEntry thread;
    // get free thread
	pthread_mutex_lock( &tPool->threadListLock ); 
    thread = list_first_entry( &tPool->threadList, 
                    struct mjThreadEntry, nodeList );
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
======================================================================
mjThreadPool_AddThread
    add thread to threadpool
======================================================================
*/
bool mjThreadPool2_AddThread( mjThreadPool2 tPool, mjThread thread )
{
    if ( !tPool ) {
        MJLOG_ERR( "tPool is null" );
        return false;
    }
    thread->private = tPool;
    INIT_LIST_HEAD( &thread->nodeList );

    pthread_mutex_lock( &tPool->threadListLock );
    list_add_tail( &thread->nodeList, &tPool->threadList );
    pthread_mutex_unlock( &tPool->threadListLock );

    return true;
}

/*
==========================================================
mjThreadPool_New
    init new thread pool
    return: NOT NULL--- mjThreadPool struct, NULL --- fail
==========================================================
*/
mjThreadPool mjThreadPool2_New() 
{
    mjThreadPool2 tPool = ( mjThreadPool2 ) calloc( 1, sizeof( struct mjThreadPool2 ) );
    if ( !tPool ) {
        MJLOG_ERR( "mjThreadPool alloc error" );
        return NULL;
    }

    pthread_mutex_init( &tPool->threadListLock, NULL ); 
	INIT_LIST_HEAD( &tPool->threadList ); 
	tPool->shutDown     = 0; 
	tPool->threadNum    = 0; 
    
    return tPool; 
} 

/* 
===========================================
mjThreadPool_delete
    destory thread pool
===========================================
*/
bool mjThreadPool2_Delete( mjThreadPool2 tPool ) 
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

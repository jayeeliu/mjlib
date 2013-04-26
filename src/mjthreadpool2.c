#include <stdlib.h>

#include "mjthreadpool2.h"
#include "mjlog.h"

/*
===========================================================================
mjThreadPool_AddWork
    add worker to thread pool
    return: 0 --- success, -1 --- fail
===========================================================================
*/ 
bool mjThreadPool2_AddWork( mjThreadPool2 tPool, mjProc Routine, void* arg ) 
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
    if ( list_empty( &tPool->freeList ) ) return false;
    // get free thread
	pthread_mutex_lock( &tPool->freeListLock ); 
    mjThreadEntry2 entry  = list_first_entry( &tPool->freeList, 
                    struct mjThreadEntry2, nodeList );
    if ( entry ) {
        list_del_init( &entry->nodeList );
    }
	pthread_mutex_unlock( &tPool->freeListLock ); 
    if ( !entry ) return false;
    // dispatch work to thread
    int ret = mjThread_AddWork( entry->thread, Routine, arg );
    if ( !ret ) {
        MJLOG_ERR( "Oops AddWork Error, Thread Lost" );
    }
    return ret;
}

static void* mjThreadPool2_ThreadFin( void* arg )
{
    mjThread thread = ( mjThread ) arg;
    mjThreadEntry2 entry = ( mjThreadEntry2) thread->private;
    // add thread to free list 
    pthread_mutex_lock( &entry->tPool->freeListLock );
    list_add_tail( &entry->nodeList, &entry->tPool->freeList );
    pthread_mutex_unlock( &entry->tPool->freeListLock ); 
    return NULL;
}

/*
==========================================================
mjThreadPool_New
    init new thread pool
    return: NOT NULL--- mjThreadPool struct, NULL --- fail
==========================================================
*/
mjThreadPool2 mjThreadPool2_New( int maxThread ) 
{
    mjThreadPool2 tPool = ( mjThreadPool2 ) calloc( 1, 
        sizeof( struct mjThreadPool2 ) + maxThread * sizeof( struct mjThreadEntry2 ) );
    if ( !tPool ) {
        MJLOG_ERR( "mjThreadPool alloc error" );
        return NULL;
    }

    tPool->maxThread    = maxThread;
    pthread_mutex_init( &tPool->freeListLock, NULL ); 
	INIT_LIST_HEAD( &tPool->freeList ); 

    for ( int i = 0; i < maxThread; i++ ) {
        tPool->threads[i].tPool = tPool;
        INIT_LIST_HEAD( &tPool->threads[i].nodeList );
        list_add_tail ( &tPool->threads[i].nodeList, &tPool->freeList );
        tPool->threads[i].thread = mjThread_New();
        mjThread_SetPrivate( tPool->threads[i].thread, &tPool->threads[i], NULL );
        mjThread_SetPrePost( tPool->threads[i].thread, NULL, mjThreadPool2_ThreadFin );
    }
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
    // free all thread
	for ( int i = 0; i < tPool->maxThread; i++ ) {
        mjThread_Delete( tPool->threads[i].thread );
    }
    // free mutex
    pthread_mutex_destroy( &tPool->freeListLock );
    // free memory
    free(tPool);
	return true; 
} 

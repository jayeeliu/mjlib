#define _GNU_SOURCE
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "mjmainsrv.h"
#include "mjconn.h"
#include "mjcomm.h"
#include "mjsock.h"
#include "mjsig.h"
#include "mjlog.h"

struct mjMainSrv_AsyncData {
    int     finNotify_r;
    int     finNotify_w;
    mjEV    ev;
    mjProc  workerRoutine;
    void*   rdata;
    mjProc  CallBack;
    void*   cdata;
};
typedef struct mjMainSrv_AsyncData* mjMainSrv_AsyncData;

/*
===============================================================================
mjMainSrv_AsyncFinCallBack
    call when asyncroutine finish
===============================================================================
*/
static void* mjMainSrv_AsyncFinCallBack( void* data ) {
    mjMainSrv_AsyncData asyncData = ( mjMainSrv_AsyncData ) data;
    int finNotify_r     = asyncData->finNotify_r;
    int finNotify_w     = asyncData->finNotify_w;
    mjEV ev             = asyncData->ev;
    mjProc CallBack     = asyncData->CallBack;
    void* cdata         = asyncData->cdata;
    char buffer[2];
    // get and clean
    read( finNotify_r, buffer, sizeof( buffer ) );
    mjEV_Del( ev, finNotify_r, MJEV_READABLE );
    close( finNotify_r );
    close( finNotify_w );
    free( asyncData );
    // run callback proc
    CallBack( cdata );
    return NULL;
}

/*
===============================================================================
mjMainSrv_AsyncRoutine
    aync run routine
===============================================================================
*/
static void* mjMainSrv_AsyncRoutine( void* data ) {
    // get data from asyncData
    mjMainSrv_AsyncData asyncData = ( mjMainSrv_AsyncData ) data;
    int finNotify_w         = asyncData->finNotify_w;
    mjProc workerRoutine    = asyncData->workerRoutine;
    void* rdata             = asyncData->rdata;
    // run Routine
    workerRoutine( rdata );
    // notify eventloop
    write( finNotify_w, "OK", 2 );
    return NULL;
}

/*
===============================================================================
mjMainSrv_Async
    run workerRoutine in threadpool, when finish call CallBack
===============================================================================
*/
bool mjMainSrv_Async( mjMainSrv mainSrv, mjProc workerRoutine, void* rdata,
            mjEV ev, mjProc CallBack, void* cdata ) {
    // sanity check
    if ( !mainSrv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }
    // alloc mjMainSrv_AsyncData
    mjMainSrv_AsyncData asyncData = ( mjMainSrv_AsyncData ) calloc ( 1,
                sizeof( struct mjMainSrv_AsyncData ) );
    if ( !asyncData ) {
        MJLOG_ERR( "AsycData alloc Error" );
        return false;
    }
    asyncData->ev               = ev;
    asyncData->workerRoutine    = workerRoutine;
    asyncData->rdata            = rdata;
    asyncData->CallBack         = CallBack;
    asyncData->cdata            = cdata;
    // alloc notify pipe and add callback event to eventloop
    int notifyFd[2];
    if ( pipe( notifyFd ) ) {
        MJLOG_ERR( "pipe alloc error" );
        free( asyncData );
        return false;
    }
    mjEV_Add( ev, notifyFd[0], MJEV_READABLE, 
            mjMainSrv_AsyncFinCallBack, asyncData );
    asyncData->finNotify_r  = notifyFd[0];
    asyncData->finNotify_w  = notifyFd[1];
    // add routine to threadpool
    if ( !mjThreadPool_AddWorkPlus( mainSrv->workerThreadPool, 
            mjMainSrv_AsyncRoutine, asyncData ) ) {
        MJLOG_ERR( "Oops async run Error" );
        // del notify event
        mjEV_Del( ev, notifyFd[0], MJEV_READABLE );
        close( notifyFd[0] );
        close( notifyFd[1] );
        free( asyncData );
        return false;
    }
    return true;
}

/*
===============================================================================
mjMainSrv_Run
    run main server. just for accept and dispatch
===============================================================================
*/
bool mjMainSrv_Run( mjMainSrv mainSrv ) {
    // sanity check
    if ( !mainSrv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }
    // set cpu affinity
    cpu_set_t cpuset;
    CPU_ZERO( &cpuset );
    CPU_SET( 0, &cpuset );
    pthread_setaffinity_np( pthread_self(), sizeof( cpu_set_t ), &cpuset );
    // accept and dispatch
    static int dispatchServer = 0;
    while ( !mainSrv->stop ) {
        int cfd = mjSock_Accept( mainSrv->sfd );
        if ( cfd < 0 ) {
            MJLOG_ERR( "mjSock_Accept Error continue" );
            continue;
        }
        dispatchServer = ( dispatchServer + 1 ) % mainSrv->serverNum;
        int ret = write( mainSrv->serverNotify[dispatchServer], 
                    &cfd, sizeof( int ) ); 
        if ( ret < 0 ) {
            MJLOG_ERR( "set socket to thread error, close" );
            mjSock_Close( cfd );
        }
    }
    return true;
}

/*
===============================================================================
mjMainSrv_New
    create new mjserver struct
===============================================================================
*/
mjMainSrv mjMainSrv_New( int sfd, mjProc serverRoutine, int workerThreadNum ) {
    // alloc server struct
    mjMainSrv mainSrv = ( mjMainSrv ) calloc ( 1, 
            sizeof( struct mjMainSrv ) );
    if ( !mainSrv ) {
        MJLOG_ERR( "mjserver create error" );
        return NULL;
    }
    // set listen socket blocking
    mjSock_SetBlocking( sfd, 1 );
    // update fileds
    mainSrv->sfd            = sfd;
    // update server
    mainSrv->serverRoutine  = serverRoutine;
    mainSrv->serverNum      = GetCPUNumber();
    if ( mainSrv->serverNum <= 0 ) {
        MJLOG_ERR( "cpu count error" );
        free( mainSrv );
        return NULL;
    }
    // create server
    int fd[2];
    for ( int i = 0; i < mainSrv->serverNum; i++ ) {
        // set serverNotify
        if ( socketpair( AF_LOCAL, SOCK_STREAM, 0, fd ) ) {
            MJLOG_ERR( "socketpair error" );
            mjMainSrv_Delete( mainSrv );
            return NULL;
        }
        mainSrv->serverNotify[i] = fd[0];
        // create new server struct and set mainServer
        mainSrv->server[i] = mjTcpSrv_New( fd[1], mainSrv->serverRoutine, 
                                MJTCPSRV_INNER );
        if ( !mainSrv->server[i] ) {
            MJLOG_ERR( "mjTcpSrv create error" );
            mjMainSrv_Delete( mainSrv );
            return NULL;
        }
        mainSrv->server[i]->mainServer = mainSrv;
        // create new thread
        mainSrv->serverThread[i] = mjThread_New();
        if ( !mainSrv->serverThread[i] ) {
            MJLOG_ERR( "mjThread create error" );
            mjMainSrv_Delete( mainSrv );
            return NULL;
        }
        mjThread_AddWork( mainSrv->serverThread[i], mjTcpSrv_Run, mainSrv->server[i],
                NULL, NULL, mjTcpSrv_Delete, mainSrv->server[i] );
        // set cpu affinity
        cpu_set_t cpuset;
        CPU_ZERO( &cpuset );
        CPU_SET( i, &cpuset );
        pthread_setaffinity_np( mainSrv->serverThread[i]->threadID, 
                sizeof( cpu_set_t ), &cpuset );
    }
    // update threadpool
    mainSrv->workerThreadNum = workerThreadNum;
    mainSrv->workerThreadPool = mjThreadPool_New( mainSrv->workerThreadNum ); 
    if ( !mainSrv->workerThreadPool ) {
        MJLOG_ERR( "threadpool create error" );
        mjMainSrv_Delete( mainSrv );
        return NULL;
    }
    return mainSrv;
}

/*
===============================================================================
mjMainSrv_Delete
    delete mjMainSrv struct
===============================================================================
*/
bool mjMainSrv_Delete( mjMainSrv mainSrv ) {
    // sanity check
    if ( !mainSrv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }
    // free fd and mjTcpSrv
    for ( int i = 0; i < mainSrv->serverNum; i++ ) {
        // free server and server thread
        if ( mainSrv->serverThread[i] ) {
            mjTcpSrv_SetStop( mainSrv->server[i], 1 );
            mjThread_Delete( mainSrv->serverThread[i] );
        }
        if ( mainSrv->serverNotify[i] ) {
            mjSock_Close( mainSrv->serverNotify[i] );
        }
    }
    // free worker threadpool
    if ( mainSrv->workerThreadPool ) {
        mjThreadPool_Delete( mainSrv->workerThreadPool );
    }
    free( mainSrv );
    return true;
}

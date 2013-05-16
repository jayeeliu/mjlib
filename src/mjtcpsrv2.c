#define _GNU_SOURCE
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "mjtcpsrv2.h"
#include "mjconn2.h"
#include "mjcomm.h"
#include "mjsock.h"
#include "mjsig.h"
#include "mjlog.h"

/*
===============================================================================
mjTcpSrv2_AcceptRoutine
    accept routine
===============================================================================
*/
static void* mjTcpSrv2_AcceptRoutine( void* arg ) {
    mjTcpSrv2 srv = ( mjTcpSrv2 ) arg;
    // read new client socket
    int cfd;
    int ret = read( srv->sfd, &cfd, sizeof( int ) );
    if ( ret < 0 || cfd < 0 ) {
        MJLOG_ERR( "Too Bad, read socket error" );
        return NULL;
    }
    // no server routine exit
    if ( !srv->Routine ) {
        MJLOG_ERR( "no server Routine found" );
        mjSock_Close( cfd );
        return NULL;
    }
    // create new mjconn
    mjConn2 conn = mjConn2_New( srv->ev, cfd );
    if ( !conn ) {
        MJLOG_ERR( "mjConn2 create error" );
        mjSock_Close( cfd );
        return NULL;
    }
    mjConn2_SetServer( conn, srv );
    srv->Routine( conn );
    return NULL;
}

/*
===============================================================================
mjTcpSrv2_Run
    run server routine in threadloop
===============================================================================
*/
static void* mjTcpSrv2_Run( void* arg ) {
    mjTcpSrv2 srv = ( mjTcpSrv2 ) arg;
    mjEV2_Run( srv->ev );
    return NULL;
}

/*
===============================================================================
mjTcpSrv2_New
    create new mjtcpsrv2 struct
===============================================================================
*/
static mjTcpSrv2 mjTcpSrv2_New( int sfd, mjProc Routine ) {
    // alloc mjTcpSrv2 struct
    mjTcpSrv2 srv = ( mjTcpSrv2 ) calloc ( 1, sizeof( struct mjTcpSrv2 ) );
    if ( !srv ) {
        MJLOG_ERR( "create server error" );
        goto failout1;
    }
    srv->sfd     = sfd;
    srv->Routine = Routine;
    // set event Loop
    srv->ev = mjEV2_New();
    if ( !srv->ev ) {
        MJLOG_ERR( "create ev error" );
        goto failout2;
    }
    // add read event
    if ( ( mjEV2_Add( srv->ev, srv->sfd, MJEV_READABLE,
            mjTcpSrv2_AcceptRoutine, srv ) ) < 0 ) {
        MJLOG_ERR( "mjev add error" );
        goto failout3;
    }
    // set signal
    mjSig_Init();
    mjSig_Register( SIGPIPE, SIG_IGN );
    return srv;

failout3:
    mjEV2_Delete( srv->ev );
failout2:
    free( srv );
failout1:
    mjSock_Close( sfd );
    return NULL; 
}

/*
===============================================================================
mjTcpSrv2_Delete
    delete mjtcpsrv2 struct
===============================================================================
*/
static bool mjTcpSrv2_Delete( mjTcpSrv2 srv ) {
    // sanity check
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }
    // free private
    if ( srv->private && srv->FreePrivate ) srv->FreePrivate( srv->private );
    mjEV2_Delete( srv->ev );
    mjSock_Close( srv->sfd );
    free( srv );
    return true;
}

static void* mjMainServer_AsyncFinCallBack( void* data ) {
    mjMainServer_AsyncData asyncData = ( mjMainServer_AsyncData ) data;
    int finNotify_r     = asyncData->finNotify_r;
    int finNotify_w     = asyncData->finNotify_w;
    mjEV2 ev            = asyncData->ev;
    mjProc CallBack     = asyncData->CallBack;
    void* cdata         = asyncData->cdata;
    char buffer[2];
    // get and clean
    read( finNotify_r, buffer, sizeof( buffer ) );
    mjEV2_Del( ev, finNotify_r, MJEV_READABLE );
    close( finNotify_r );
    close( finNotify_w );
    free( asyncData );
    // run callback proc
    CallBack( cdata );
    return NULL;
}

/*
===============================================================================
mjMainServer_AsyncRoutine
    aync run routine
===============================================================================
*/
static void* mjMainServer_AsyncRoutine( void* data ) {
    // get data from asyncData
    mjMainServer_AsyncData asyncData = ( mjMainServer_AsyncData ) data;
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
mjMainServer_Async
    run workerRoutine in threadpool, when finish call CallBack
===============================================================================
*/
bool mjMainServer_Async( mjMainServer srv, mjProc workerRoutine, void* rdata,
            mjEV2 ev, mjProc CallBack, void* cdata ) {
    // sanity check
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }
    // alloc mjMainServer_AsyncData
    mjMainServer_AsyncData asyncData = ( mjMainServer_AsyncData ) calloc ( 1,
                sizeof( struct mjMainServer_AsyncData ) );
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
    mjEV2_Add( ev, notifyFd[0], MJEV_READABLE, 
            mjMainServer_AsyncFinCallBack, asyncData );
    asyncData->finNotify_r  = notifyFd[0];
    asyncData->finNotify_w  = notifyFd[1];
    // add routine to threadpool
    if ( !mjThreadPool2_AddWork( srv->workerThreadPool, 
            mjMainServer_AsyncRoutine, asyncData ) ) {
        if ( !mjThread_RunOnce( mjMainServer_AsyncRoutine, asyncData ) ) {
            MJLOG_ERR( "Oops async run Error" );
            // del notify event
            mjEV2_Del( ev, notifyFd[0], MJEV_READABLE );
            close( notifyFd[0] );
            close( notifyFd[1] );
            free( asyncData );
            return false;
        }
    }
    return true;
}

/*
===============================================================================
mjMainServer_Run
    run main server. just for accept and dispatch
===============================================================================
*/
bool mjMainServer_Run( mjMainServer srv ) {
    // sanity check
    if ( !srv ) {
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
    while ( !srv->stop ) {
        int cfd = mjSock_Accept( srv->sfd );
        if ( cfd < 0 ) {
            MJLOG_ERR( "mjSock_Accept Error continue" );
            continue;
        }
        dispatchServer = ( dispatchServer + 1 ) % srv->serverNum;
        int ret = write( srv->serverNotify[dispatchServer], 
                    &cfd, sizeof( int ) ); 
        if ( ret < 0 ) {
            MJLOG_ERR( "set socket to thread error, close" );
            mjSock_Close( cfd );
        }
    }
    return true;
}

/*
================================================================================
mjMainServer_New
    create new mjserver struct
===============================================================================
*/
mjMainServer mjMainServer_New( int sfd, mjProc serverRoutine, int workerThreadNum ) {
    // alloc server struct
    mjMainServer srv = ( mjMainServer ) calloc ( 1, 
            sizeof( struct mjMainServer ) );
    if ( !srv ) {
        MJLOG_ERR( "mjserver create error" );
        return NULL;
    }
    // set listen socket blocking
    mjSock_SetBlocking( sfd, 1 );
    // update fileds
    srv->sfd            = sfd;
    // update server
    srv->serverRoutine  = serverRoutine;
    srv->serverNum      = GetCPUNumber();
    if ( srv->serverNum <= 0 ) {
        MJLOG_ERR( "cpu count error" );
        free( srv );
        return NULL;
    }
    // create server
    int fd[2];
    for ( int i = 0; i < srv->serverNum; i++ ) {
        // set serverNotify
        if ( socketpair( AF_LOCAL, SOCK_STREAM, 0, fd ) ) {
            MJLOG_ERR( "socketpair error" );
            mjMainServer_Delete( srv );
            return NULL;
        }
        srv->serverNotify[i] = fd[0];
        // create new server struct and set mainServer
        srv->server[i] = mjTcpSrv2_New( fd[1], srv->serverRoutine );
        if ( !srv->server[i] ) {
            MJLOG_ERR( "mjTcpSrv2 create error" );
            mjMainServer_Delete( srv );
            return NULL;
        }
        srv->server[i]->mainServer = srv;
        // create new thread
        srv->serverThread[i] = mjThread_NewLoop( mjTcpSrv2_Run, 
                srv->server[i] );
        if ( !srv->serverThread[i] ) {
            MJLOG_ERR( "mjThread create error" );
            mjMainServer_Delete( srv );
            return NULL;
        }
        // set cpu affinity
        cpu_set_t cpuset;
        CPU_ZERO( &cpuset );
        CPU_SET( i, &cpuset );
        pthread_setaffinity_np( srv->serverThread[i]->threadID, 
                sizeof( cpu_set_t ), &cpuset );
    }
    // update threadpool
    srv->workerThreadNum = workerThreadNum;
    srv->workerThreadPool = mjThreadPool2_New( srv->workerThreadNum ); 
    if ( !srv->workerThreadPool ) {
        MJLOG_ERR( "threadpool create error" );
        mjMainServer_Delete( srv );
        return NULL;
    }
    return srv;
}

/*
===============================================================================
mjMainServer_Delete
    delete mjMainServer struct
===============================================================================
*/
bool mjMainServer_Delete( mjMainServer srv ) {
    // sanity check
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }
    // free fd and mjTcpSrv2
    for ( int i = 0; i < srv->serverNum; i++ ) {
        // free server thread
        if ( srv->serverThread[i] ) {
            mjThread_Delete( srv->serverThread[i] );
        }
        if ( srv->serverNotify[i] ) {
            mjSock_Close( srv->serverNotify[i] );
        }
        if ( srv->server[i] ) {
            mjTcpSrv2_Delete( srv->server[i] );
        }
    }
    // free worker threadpool
    if ( srv->workerThreadPool ) {
        mjThreadPool2_Delete( srv->workerThreadPool );
    }
    free(srv);
    return true;
}

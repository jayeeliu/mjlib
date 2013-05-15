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

static void* mjTcpSrv2_Run( void* arg ) {
    mjTcpSrv2 srv = ( mjTcpSrv2 ) arg;
    mjEV2_Run( srv->ev );
    return NULL;
}

static mjTcpSrv2 mjTcpSrv2_New( int sfd, mjProc Routine ) {
    // alloc mjTcpSrv2 struct
    mjTcpSrv2 srv = ( mjTcpSrv2 ) calloc ( 1, 
            sizeof( struct mjTcpSrv2 ) );
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

static bool mjTcpSrv2_Delete( mjTcpSrv2 srv ) {
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }
    if ( srv->private && srv->FreePrivate ) {
        srv->FreePrivate( srv->private );
    }
    mjEV2_Delete( srv->ev );
    mjSock_Close( srv->sfd );
    free( srv );
    return true;
}

bool mjServer_Run( mjServer srv ) {
    // sanity check
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }
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
        write( srv->serverNotify[dispatchServer], &cfd, sizeof( int ) ); 
    }
    return true;
}

/*
=========================================================
mjServer_New
    create new mjserver struct
=========================================================
*/
mjServer mjServer_New( int sfd, mjProc serverRoutine ) {
    // alloc server struct
    mjServer srv = ( mjServer ) calloc ( 1, 
            sizeof( struct mjServer ) );
    if ( !srv ) {
        MJLOG_ERR( "mjserver create error" );
        return NULL;
    }
    // set listen socket blocking
    mjSock_SetBlocking( sfd, 1 );
    // update fileds
    srv->sfd            = sfd;
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
        if ( socketpair( AF_LOCAL, SOCK_STREAM, 0, fd ) ) {
            MJLOG_ERR( "socketpair error" );
            mjServer_Delete( srv );
            return NULL;
        }
        srv->serverNotify[i] = fd[0];
        srv->server[i] = mjTcpSrv2_New( fd[1], srv->serverRoutine );
        if ( !srv->server[i] ) {
            MJLOG_ERR( "mjTcpSrv2 create error" );
            mjServer_Delete( srv );
            return NULL;
        }
        srv->serverThread[i] = mjThread_NewLoop( mjTcpSrv2_Run, srv->server[i] );
        if ( !srv->serverThread[i] ) {
            MJLOG_ERR( "mjThread create error" );
            mjServer_Delete( srv );
            return NULL;
        }
        cpu_set_t cpuset;
        CPU_ZERO( &cpuset );
        CPU_SET( i, &cpuset );
        pthread_setaffinity_np( srv->serverThread[i]->threadID, sizeof( cpu_set_t ), &cpuset );
    }
    return srv;
}

bool mjServer_Delete( mjServer srv ) {
    // sanity check
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }
    // free fd and mjTcpSrv2
    for ( int i = 0; i < srv->serverNum; i++ ) {
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
    free(srv);
    return true;
}

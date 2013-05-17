#include <stdlib.h>
#include <unistd.h>
#include "mjtcpsrv2.h"
#include "mjconn.h"
#include "mjsig.h"
#include "mjsock.h"
#include "mjlog.h"

/*
===============================================================================
mjTcpSrv2_AcceptRoutine
    accept routine
===============================================================================
*/
void* mjTcpSrv2_AcceptRoutine( void* arg ) {
    mjTcpSrv2 srv = ( mjTcpSrv2 ) arg;
    // read new client socket
    int cfd;
    if ( srv->type == MJTCPSRV_STANDALONE ) {
        // standalone mode, accept new socket
        cfd = mjSock_Accept( srv->sfd );
        if ( cfd < 0 ) return NULL;
    } else if ( srv->type == MJTCPSRV_INNER ) {
        // innner mode, read new socket
        int ret = read( srv->sfd, &cfd, sizeof( int ) );
        if ( ret < 0 || cfd < 0 ) {
            MJLOG_ERR( "Too Bad, read socket error" );
            return NULL;
        }
    } else {
        MJLOG_ERR( "mjTcpSrv2 type error" );
        return NULL;
    }
    // no server routine exit
    if ( !srv->Routine ) {
        MJLOG_ERR( "no server Routine found" );
        mjSock_Close( cfd );
        return NULL;
    }
    // create new mjconn
    mjConn conn = mjConn_New( srv->ev, cfd );
    if ( !conn ) {
        MJLOG_ERR( "mjConn create error" );
        mjSock_Close( cfd );
        return NULL;
    }
    mjConn_SetServer( conn, srv );
    srv->Routine( conn );
    return NULL;
}

/*
===============================================================================
mjTcpSrv2_Run
    run mjTcpSrv2
===============================================================================
*/
void* mjTcpSrv2_Run( void* arg ) {
    // sanity check
    if ( !arg ) {
        MJLOG_ERR( "server is null" );
        return NULL;
    }
    mjTcpSrv2 srv = ( mjTcpSrv2 ) arg;
    // enter loop
    while ( !srv->stop ) {
        mjEV_Run( srv->ev );
        if ( srv->type == MJTCPSRV_STANDALONE ) {
            mjSig_ProcessQueue();
        }
    }
    return NULL;
}

/*
===============================================================================
mjTcpSrv2_SetPrivate
    set private data and proc
===============================================================================
*/
bool mjTcpSrv2_SetPrivate( mjTcpSrv2 srv, void* private,
                mjProc FreePrivate ) {
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }
    srv->private     = private;
    srv->FreePrivate = FreePrivate;
    return true;
}

/*
===============================================================================
mjTcpSrv2_SetStop
    set mjTcpSrv stop
===============================================================================
*/
bool mjTcpSrv2_SetStop( mjTcpSrv2 srv, int value ) {
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }
    srv->stop = ( value == 0 ) ? 0 : 1;
    return true;
}

/*
===============================================================================
mjTcpSrv2_New
    alloc mjTcpSrv2 struct
===============================================================================
*/
mjTcpSrv2 mjTcpSrv2_New( int sfd, mjProc Routine, int type ) {
    // alloc mjTcpSrv2 struct
    mjTcpSrv2 srv = ( mjTcpSrv2 ) calloc ( 1, sizeof( struct mjTcpSrv2 ) );    
    if ( !srv ) {
        MJLOG_ERR( "create server error" );
        goto failout1;
    }
    // check type
    if ( type != MJTCPSRV_STANDALONE && type != MJTCPSRV_INNER ) {
        MJLOG_ERR( "server type error" );
        goto failout2;
    }
    // set sfd nonblock
    mjSock_SetBlocking( srv->sfd, 0 );
    // set fields
    srv->sfd        = sfd;
    srv->type       = type;
    srv->Routine    = Routine;
    // set event Loop
    srv->ev = mjEV_New();
    if ( !srv->ev ) {
        MJLOG_ERR( "create ev error" );
        goto failout2;
    }
    // add read event
    if ( ( mjEV_Add( srv->ev, srv->sfd, MJEV_READABLE,
            mjTcpSrv2_AcceptRoutine, srv ) ) < 0 ) {
        MJLOG_ERR( "mjev add error" );
        goto failout3;
    }
    // set signal
    mjSig_Init();
    mjSig_Register( SIGPIPE, SIG_IGN );
    return srv;

failout3:
    mjEV_Delete( srv->ev );
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
void* mjTcpSrv2_Delete( void* arg ) {
    mjTcpSrv2 srv = ( mjTcpSrv2 ) arg;
    // sanity check
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return NULL;
    }
    // free private
    if ( srv->private && srv->FreePrivate ) srv->FreePrivate( srv->private );
    mjEV_Delete( srv->ev );
    mjSock_Close( srv->sfd );
    free( srv );
    return NULL;
}

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "mjlog.h"
#include "mjsock.h"
#include "mjtcpsrv.h"
#include "mjconn.h"
#include "mjsig.h"

static void mjTcpSrv_AcceptHandler( void* data )
{
    mjTcpSrv srv = ( mjTcpSrv )data;

    // accept, create new client socket 
    int cfd = mjSock_Accept( srv->sfd );
    if ( cfd < 0 ) {
//        MJLOG_ERR( "mjSock_Accept error: %s", strerror( errno ) );
        return;
    }
    // no server Handler, exit 
    if ( !srv->Handler ) {
        MJLOG_WARNING( "no server Handler found" );
        mjSock_Close( cfd );
        return;
    }
    // create new action 
    mjConn conn = mjConn_New( srv, srv->ev, cfd );
    if ( !conn ) {
        MJLOG_ERR( "mjConn create error" );
        mjSock_Close( cfd );
        return;
    }
    // run Handler, conn is parameter
    srv->Handler( conn );
}

/*
========================================
mjTcpSrv_Run
    enter event loop
========================================
*/
bool mjTcpSrv_Run( mjTcpSrv srv )
{
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }
    // run InitSrv
    if ( srv->InitSrv ) {
        srv->InitSrv( srv );
    }
    // enter loop
    while ( !srv->stop ) {
        mjEV_Run( srv->ev );
        mjSig_ProcessQueue();
    }
    return true;
}

/*
==========================================================
mjTcpSrv_SetHandler
    set server handler, called when new conn, create
    conn is the parameter
==========================================================
*/
bool mjTcpSrv_SetHandler( mjTcpSrv srv, mjproc* Handler )
{
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }
    srv->Handler = Handler;
    return true;
}

/*
=========================================================================
mjTcpSrv_SetSrvProc
    set server init and exit proc. called when server begin and exit.
    srv is the parameter
=========================================================================
*/
bool mjTcpSrv_SetSrvProc( mjTcpSrv srv, mjproc* InitSrv, mjproc* ExitSrv )
{
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }
    srv->InitSrv = InitSrv;
    srv->ExitSrv = ExitSrv;
    return true;
}

/*
============================================================================
mjTcpSrv_SetPrivate
    set server private struct and free private function
============================================================================
*/
bool mjTcpSrv_SetPrivate( mjTcpSrv srv, void* private, mjproc* FreePrivate )
{
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }
    srv->private     = private;
    srv->FreePrivate = FreePrivate;
    return true;
}

/*
===================================================
mjTcpSrv_SetStop
    set stop status
===================================================
*/
bool mjTcpSrv_SetStop( mjTcpSrv srv, int value )
{
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }
    srv->stop = ( value == 0 ) ? 0 : 1;
    return true;
}

static bool mjTcpSrv_EnableAccept( mjTcpSrv srv )
{
    int ret = mjEV_Add( srv->ev, srv->sfd, MJEV_READABLE,
                    mjTcpSrv_AcceptHandler, srv );
    if ( ret < 0 ) {
        MJLOG_ERR( "mjev add error" );
        return false;
    }
    return true;
}

/*
static bool mjTcpSrv_DisableAccept( mjTcpSrv srv )
{
    int ret = mjEV_Del( srv->ev, srv->sfd, MJEV_READABLE );
    if ( ret < 0 ) {
        MJLOG_ERR( "mjev del error" );
        return false;
    }
    return true;
}
*/

/*
=========================================
mjTcpSrv_new
    create new tcpserver
=========================================
*/
mjTcpSrv mjTcpSrv_New( int sfd )
{
    // alloc mjserver_tcp struct
    mjTcpSrv srv = ( mjTcpSrv ) calloc( 1, sizeof( struct mjTcpSrv ) );
    if ( !srv ) {
        MJLOG_ERR( "create server error" );
        goto failout1;
    }
    // set server filed
    srv->sfd     = sfd;  
    mjSock_SetBlocking( srv->sfd, 0 );
    // server not stop
    srv->stop    = 0;
    // init server loop
    srv->ev = mjEV_New();
    if ( !srv->ev ) {
        MJLOG_ERR( "create ev error" );
        goto failout2;
    }

    // enable accept
    if ( !mjTcpSrv_EnableAccept( srv ) ) {
        MJLOG_ERR( "mjev add error" );
        goto failout3;
    }
   
    srv->Handler     = NULL;
    srv->InitSrv     = NULL;
    srv->ExitSrv     = NULL;
    srv->private     = NULL;
    srv->FreePrivate = NULL;
       
    // init signal 
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
=========================================
mjTcpSrv_Delete
    call ExitSrv, delete eventloop
    close server socket
=========================================
*/
bool mjTcpSrv_Delete( mjTcpSrv srv )
{
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }
    if ( srv->ExitSrv ) {
        srv->ExitSrv( srv );
    }
    if ( srv->private && srv->FreePrivate ) {
        srv->FreePrivate( srv->private );
    }

    mjEV_Delete( srv->ev );
    mjSock_Close( srv->sfd );
    free( srv );
    return true;
}

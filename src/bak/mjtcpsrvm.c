#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "mjlog.h"
#include "mjsock.h"
#include "mjtcpsrvm.h"
#include "mjconn.h"
#include "mjsig.h"

static void* mjTcpSrvM_AcceptHandler( void* data )
{
    mjTcpSrvM srv = ( mjTcpSrvM )data;

    int cfd;
    read( srv->acceptThreadNotifyRead, &cfd, sizeof( int ) );
    
    // no server Handler, exit 
    if ( !srv->Handler ) {
        MJLOG_WARNING( "no server Handler found" );
        mjSock_Close( cfd );
        return NULL;
    }
    // create new action 
    mjConn conn = mjConn_New( srv->ev, cfd );
    if ( !conn ) {
        MJLOG_ERR( "mjConn create error" );
        mjSock_Close( cfd );
        return NULL;
    }
    // set conn server
    mjConn_SetServer( conn, srv );
    // run Handler, conn is parameter
    srv->Handler( conn );
    return NULL;
}

/*
========================================
mjTcpSrvM_Run
    enter event loop
========================================
*/
bool mjTcpSrvM_Run( mjTcpSrvM srv )
{
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }

    // run InitSrv
    if ( srv->InitSrv ) {
        srv->InitSrv( srv );
    }
    
    while( !srv->stop ) {
        mjEV_Run( srv->ev );
        mjSig_ProcessQueue();
    }
    return true;
}

/*
==========================================================
mjTcpSrvM_SetHandler
    set server handler, called when new conn, create
    conn is the parameter
==========================================================
*/
bool mjTcpSrvM_SetHandler( mjTcpSrvM srv, mjProc Handler )
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
mjTcpSrvM_SetSrvProc
    set server init and exit proc. called when server begin and exit.
    srv is the parameter
=========================================================================
*/
bool mjTcpSrvM_SetSrvProc( mjTcpSrvM srv, mjProc InitSrv, mjProc ExitSrv )
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
mjTcpSrvM_SetPrivate
    set server private struct and free private function
============================================================================
*/
bool mjTcpSrvM_SetPrivate( mjTcpSrvM srv, void* private, mjProc FreePrivate )
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
mjTcpSrvM_SetStop
    set stop status
===================================================
*/
bool mjTcpSrvM_SetStop( mjTcpSrvM srv, int value )
{
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }
    srv->stop = ( value == 0 ) ? 0 : 1;
    return true;
}

static void* mjTcpSrvM_Thread( void* arg )
{
    mjTcpSrvM srv = ( mjTcpSrvM )arg;

    // enter loop
    while ( !srv->stop ) {
        // accept, create new client socket 
        int cfd = mjSock_Accept( srv->sfd );
        if ( cfd < 0 ) continue;
        //notify thread
        write( srv->acceptThreadNotifyWrite, &cfd, sizeof( int ) ); 
    }

    return NULL;
}

/*
=========================================
mjTcpSrvM_new
    create new tcpserver
=========================================
*/
mjTcpSrvM mjTcpSrvM_New( int sfd )
{
    // alloc mjserver_tcp struct
    mjTcpSrvM srv = ( mjTcpSrvM ) calloc( 1, sizeof( struct mjTcpSrvM ) );
    if ( !srv ) {
        MJLOG_ERR( "create server error" );
        goto failout1;
    }
    // set server filed
    srv->sfd     = sfd;  
    // server not stop
    srv->stop    = 0;
    // init server loop
    srv->ev = mjEV_New();
    if ( !srv->ev ) {
        MJLOG_ERR( "create ev error" );
        goto failout2;
    }
    // set handler 
    srv->Handler     = NULL;
    srv->InitSrv     = NULL;
    srv->ExitSrv     = NULL;
    srv->private     = NULL;
    srv->FreePrivate = NULL;

    // init signal 
    mjSig_Init();
    mjSig_Register( SIGPIPE, SIG_IGN );

    // create thread notify fd
    int notify[2];
    int ret = pipe( notify );
    if ( ret < 0 ) {
        MJLOG_ERR( "pipe error" );
        goto failout3;
    }
    srv->acceptThreadNotifyRead   = notify[0];
    srv->acceptThreadNotifyWrite  = notify[1];

    // add NotifyReadHandler
    mjEV_Add( srv->ev, srv->acceptThreadNotifyRead, MJEV_READABLE,
            mjTcpSrvM_AcceptHandler, srv );
    // create worker thread
    pthread_create( &srv->acceptThread, NULL, mjTcpSrvM_Thread, srv );
       
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
mjTcpSrvM_Delete
    call ExitSrv, delete eventloop
    close server socket
=========================================
*/
bool mjTcpSrvM_Delete( mjTcpSrvM srv )
{
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }
    // exit acceptThread
    pthread_join( srv->acceptThread, NULL );
    close( srv->acceptThreadNotifyRead );
    close( srv->acceptThreadNotifyWrite );

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

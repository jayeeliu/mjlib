#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "mjlog.h"
#include "mjsock.h"
#include "mjtcpsrvt.h"
#include "mjconnb.h"
#include "mjsig.h"
#include "mjthread.h"

/*
=========================================================
mjTcpSrvT_AcceptHandler
    accept client fd and run
=========================================================
*/
static void mjTcpSrvT_AcceptHandler( void* arg )
{
    mjTcpSrvT srv = ( mjTcpSrvT ) arg;

    // accept, create new client socket
    int cfd = mjSock_Accept( srv->sfd );
    if ( cfd < 0 ) {
        MJLOG_ERR( "mjSock_Accept error: %d", errno );
        return;
    }
    // no server handler, exit 
    if ( !srv->Handler ) {
        MJLOG_WARNING( "no server handler found" );
        mjSock_Close( cfd );
        return;
    }
    // create new connection
    mjConnB conn = mjConnB_New( cfd );
    if ( !conn ) {
        MJLOG_ERR( "mjtcpconn create error" );
        mjSock_Close( cfd );
        return;
    }
    mjConnB_SetServer( conn, srv );

    // add worker to threadpool
    if ( srv->tpool && mjThreadPool_AddWorker( srv->tpool, 
                srv->Handler, conn ) ) return;

    mjThread_RunOnce( srv->Handler, conn );
}

/*
=========================================
mjTcpSrvT_Run
    run mjtcpsrv using thread
=========================================
*/
bool mjTcpSrvT_Run( mjTcpSrvT srv )
{
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }

    if ( srv->InitSrv ) {
        srv->InitSrv( srv );
    }

    while ( !srv->stop ) {
        mjEV_Run( srv->ev );
        mjSig_ProcessQueue();
    }
    return true;
}

/*
==============================================================
mjTcpSrvT_SetHandler
    set running worker, accept client fd and run it.
    mjConnB as parameter
==============================================================
*/
bool mjTcpSrvT_SetHandler( mjTcpSrvT srv, mjthread* Handler )
{
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }
    srv->Handler = Handler;
    return true;
}

/*
============================================================================
mjTcpSrvT_SetPrivate
    set server private
============================================================================
*/
bool mjTcpSrvT_SetPrivate( mjTcpSrvT srv, void* private, mjproc* FreePrivate )
{
    if ( !srv ) {
        MJLOG_ERR( "srv is null" );
        return false;
    }
    srv->private        = private;
    srv->FreePrivate    = FreePrivate;
    return true;
}

/*
==========================================================================
mjTcpSrvT_SetSrvProc
    set server init and exit proc
==========================================================================
*/
bool mjTcpSrvT_SetSrvProc( mjTcpSrvT srv, mjproc* InitSrv, mjproc* ExitSrv )
{
    if ( !srv ) {
        MJLOG_ERR( "srv is null" );
        return false;
    }
    srv->InitSrv    = InitSrv;
    srv->ExitSrv    = ExitSrv;
    return true;
}

/*
===================================================
mjTcpSrvT_SetStop
    set server stop value
===================================================
*/
bool mjTcpSrvT_SetStop( mjTcpSrvT srv, int value )
{
    if ( !srv ) {
        MJLOG_ERR( "srv is null" );
        return false;
    }
    srv->stop = ( value == 0 ) ? 0 : 1;
    return true;
}

/*
================================================
mjTcpSrvT_New
    create new tcp thread pool server
    threadnum: threadnum of the pool, 0 for none
    return:  NULL -- create error, close sfd
            other -- success
================================================
*/
mjTcpSrvT mjTcpSrvT_New( int sfd, int threadNum )
{
    // alloc mjserver_tcp struct 
    mjTcpSrvT srv = ( mjTcpSrvT ) calloc( 1, sizeof( struct mjTcpSrvT ) );
    if ( !srv ) {
        MJLOG_ERR( "create server error" );
        goto failout1;
    }
    srv->sfd         = sfd;          //  set server socket 
    srv->stop        = 0;            //  not stop 
    mjSock_SetBlocking( srv->sfd, 0 );
    // init server loop
    srv->ev = mjEV_New(); 
    if ( !srv->ev ) {
        MJLOG_ERR( "create ev error" );
        goto failout2;
    }
    // add accept event
    if ( mjEV_Add( srv->ev, sfd, MJEV_READABLE, 
            mjTcpSrvT_AcceptHandler, srv ) < 0 ) {
        MJLOG_ERR( "mjev add error" );
        goto failout3;
    }
    // create threadpool
    srv->tpool = NULL;
    if ( threadNum > 0 ) {
        srv->tpool = mjThreadPool_New( threadNum );
        if ( !srv->tpool ) {
            MJLOG_ERR( "create threadpool error" );
            goto failout3;
        }
    }
    srv->Handler     = NULL; 
    
    srv->InitSrv     = NULL;
    srv->ExitSrv     = NULL;
    srv->private     = NULL;
    srv->FreePrivate = NULL;

    // init signal handler
    mjSig_Init();
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
===========================================
mjTcpSrvT_Delete
    delete mjTcpSrvT server 
===========================================
*/
bool mjTcpSrvT_Delete( mjTcpSrvT srv )
{
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }

    if ( srv->tpool ) {
        mjThreadPool_Delete( srv->tpool );
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

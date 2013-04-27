#include <stdlib.h>
#include <errno.h>
#include "mjlog.h"
#include "mjsock.h"
#include "mjtcpsrvtp.h"
#include "mjconnb.h"
#include "mjthread.h"
#include "mjsig.h"

bool mjTcpSrvTP_Run( mjTcpSrvTP srv )
{
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }

    while ( !srv->stop ) {
        mjSig_ProcessQueue();

        int cfd = mjSock_Accept( srv->sfd );
        if ( cfd < 0 ) {
            if ( errno == EINTR ) continue;
            MJLOG_ERR( "mjSock_Accept Error" );
            return false;
        }

        if ( !srv->Handler ) {
            mjSock_Close( cfd );
            continue;
        }
        mjConnB conn = mjConnB_New( cfd );
        if ( !conn ) {
            MJLOG_ERR( "mjConnB create error" );
            continue;
        }
        if ( srv->tPool ) {
            if ( mjThreadPool_AddWorker( srv->tPool, srv->Handler, conn ) ) 
                continue;
        }
        mjThread_RunOnce( srv->Handler, conn );
    }
    return true;
}

bool mjTcpSrvTP_SetHandler( mjTcpSrvTP srv, mjProc Handler )
{
    if ( !srv ) {   
        MJLOG_ERR( "server is null" );
        return false;
    }
    srv->Handler = Handler;
    return true;
}

bool mjTcpSrvTP_SetStop( mjTcpSrvTP srv, int value )
{
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    } 
    srv->stop = ( value == 0 ) ? 0 : 1;
    return true;
}

/*
============================================================
mjTcpSrvTP_New
    create new tcpserver accept-threadpool
============================================================
*/
mjTcpSrvTP mjTcpSrvTP_New( int sfd, int threadNum )
{
    mjTcpSrvTP srv = ( mjTcpSrvTP ) calloc( 1, sizeof( struct mjTcpSrvTP ) );
    if ( !srv ) {
        MJLOG_ERR( "create server error" );
        goto failout1;
    }
    srv->sfd        = sfd;
    mjSock_SetBlocking( srv->sfd, 1 );
    // create thread pool
    if ( threadNum > 0 ) {
        srv->tPool = mjThreadPool_New( threadNum );
        if ( !srv->tPool ) {
            MJLOG_ERR( "create threadpool error" );
            goto failout2;
        }
    }

    mjSig_Init();
    return srv;

failout2:
    free( srv );
failout1:
    mjSock_Close( sfd );
    return NULL;
}

/*
============================================
mjTcpSrvTP_Delete
    delete server
============================================
*/
bool mjTcpSrvTP_Delete( mjTcpSrvTP srv )
{
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }

    if ( srv->tPool ) {
        mjThreadPool_Delete( srv->tPool );
    }

    mjSock_Close( srv->sfd );
    free( srv );
    return true;
}

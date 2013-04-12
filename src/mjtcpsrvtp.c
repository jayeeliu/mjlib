#include <stdlib.h>
#include "mjlog.h"
#include "mjsock.h"
#include "mjtcpsrvtp.h"
#include "mjconnb.h"

bool mjTcpSrvTP_Run( mjTcpSrvTP srv )
{
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }

    while ( !srv->stop ) {
        // TODO: signal and return value
        int cfd = mjSock_Accept( srv->sfd );
        if ( !srv->Handler ) {
            mjSock_Close( cfd );
            continue;
        }
        mjConnB conn = mjConnB_New( cfd );
        if ( srv->tpool ) {
            if ( mjThreadPool_AddWorker( srv->tpool, srv->Handler, conn ) ) 
                continue;
        }
        pthread_t tid;
        pthread_create( &tid, NULL, srv->Handler, conn );
        pthread_detach( tid );
    }
    return true;
}

bool mjTcpSrvTP_SetHandler( mjTcpSrvTP srv, mjthread* Handler )
{
    if ( !srv ) {   
        MJLOG_ERR( "server is null" );
        return false;
    }
    srv->Handler = Handler;
    return true;
}

mjTcpSrvTP mjTcpSrvTP_New( int sfd, int threadNum )
{
    mjTcpSrvTP srv = ( mjTcpSrvTP ) calloc( 1, sizeof( struct mjTcpSrvTP ) );
    if ( !srv ) {
        MJLOG_ERR( "create server error" );
        goto failout1;
    }

    srv->sfd        = sfd;
    srv->stop       = 0;
    srv->tpool      = NULL;
    srv->Handler    = NULL;

    mjSock_SetBlocking( srv->sfd, 1 );

    if ( threadNum > 0 ) {
        srv->tpool = mjThreadPool_New( threadNum );
        if ( !srv->tpool ) {
            MJLOG_ERR( "create threadpool error" );
            goto failout2;
        }
    }

    return srv;

failout2:
    free( srv );
failout1:
    mjSock_Close( sfd );
    return NULL;
}

bool mjTcpSrvTP_Delete( mjTcpSrvTP srv )
{
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }

    if ( srv->tpool ) {
        mjThreadPool_Delete( srv->tpool );
    }
    mjSock_Close( srv->sfd );
    free( srv );
    return true;
}

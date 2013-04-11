#include <stdlib.h>
#include "mjlog.h"
#include "mjsock.h"
#include "mjtcpsrvtp.h"

mjTcpSrvTP mjTcpSrvTP_New( int sfd, int threadNum )
{
    mjTcpSrvTP srv = ( mjTcpSrvTP ) calloc( 1, sizeof( struct mjTcpSrvTP ) );
    if ( !srv ) {
        MJLOG_ERR( "create server error" );
        goto failout1;
    }
    srv->sfd    = sfd;
    mjSock_SetBlocking( srv->sfd, 1 );
    srv->stop   = 0;
    srv->tpool  = NULL;
    if ( threadNum > 0 ) {
        srv->tpool = mjThreadPool_New( threadNum );
        if ( !srv->tpool ) {
            MJLOG_ERR( "create threadpool error" );
            goto failout2;
        }
    }

failout2:
    free( srv );
failout1:
    mjSock_Close( sfd );
    return NULL;
}

bool mjTcpSrvTP_Delete( mjTcpSrvTP srv )
{
}

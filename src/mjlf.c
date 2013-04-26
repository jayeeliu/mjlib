#include <stdlib.h>
#include "mjlog.h"
#include "mjlf.h"
#include "mjsock.h"

static void* mjLF_Routine( void* arg )
{
    mjLF server = ( mjLF ) arg;

    while ( 1 ) {
        int cfd = mjSock_Accept( server->port );
        if ( cfd < 0 ) continue;
        // invoke follows
        mjThreadPool2_AddWork( server->tPool, mjLF_Routine, server );

        if ( server->Routine ) {
            server->Routine( &cfd );
        }
        break;
    }
    return NULL; 
}

mjLF mjLF_New( mjProc Routine, int maxThread, int port )
{
    mjLF server = ( mjLF ) calloc( 1, sizeof( struct mjLF ) );
    if ( !server ) {
        MJLOG_ERR( "server create errror" );
        return NULL;
    }

    server->tPool = mjThreadPool2_New( maxThread );
    if ( !server->tPool ) {
        MJLOG_ERR( "mjthreadpool create error" );
        free( server );
        return NULL;
    }
    server->port    = port;
    server->Routine = Routine;
    mjThreadPool2_AddWork( server->tPool, mjLF_Routine, server );

    return server;
}

bool mjLF_Delete( mjLF server )
{
    if ( !server ) {
        MJLOG_ERR( "server is null" );
        return false;
    }

    mjThreadPool2_Delete( server->tPool );
    return true;
}

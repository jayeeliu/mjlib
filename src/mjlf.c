#include <stdlib.h>
#include "mjconnb.h"
#include "mjlog.h"
#include "mjlf.h"
#include "mjsock.h"

/*
===============================================
mjLF_Routine
    Routine Run
===============================================
*/
static void* mjLF_Routine( void* arg )
{
    mjLF server = ( mjLF ) arg;

    while ( 1 ) {
        int cfd = mjSock_Accept( server->sfd );
        if ( cfd < 0 ) continue;
        // invoke follows
        int ret = mjThreadPool2_AddWork( server->tPool, mjLF_Routine, server );
        if ( !ret ) mjThread_RunOnce( mjLF_Routine, server );

        if ( server->Routine ) {
            mjConnB conn = mjConnB_New( cfd );
            server->Routine( conn );
        } else {
            close( cfd );
        }
        break;
    }
    return NULL; 
}

/*
==========================================================
mjLF_New
    create mjLF struct
==========================================================
*/
mjLF mjLF_New( mjProc Routine, int maxThread, int sfd )
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
    server->sfd     = sfd;
    server->Routine = Routine;
    mjThreadPool2_AddWork( server->tPool, mjLF_Routine, server );
    return server;
}

/*
==========================================
mjLF_Delete
    Delete server
==========================================
*/
bool mjLF_Delete( mjLF server )
{
    if ( !server ) {
        MJLOG_ERR( "server is null" );
        return false;
    }

    mjThreadPool2_Delete( server->tPool );
    return true;
}

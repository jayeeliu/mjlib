#include <stdlib.h>
#include "mjconnb.h"
#include "mjlog.h"
#include "mjlf.h"
#include "mjsock.h"
#include "mjsig.h"

/*
===============================================
mjLF_Routine
    Routine Run
===============================================
*/
static void* mjLF_Routine( void* arg )
{
    mjLF server = ( mjLF ) arg;
    int cfd;
    // leader run this
    while ( 1 ) {
        cfd = mjSock_Accept( server->sfd );
        if ( cfd < 0 ) {
            MJLOG_ERR("mjSock_Accept Error");
            continue;
        }
        break;
    }
    // choose a new leader
    int ret = mjThreadPool2_AddWork( server->tPool, mjLF_Routine, server );
    if ( !ret ) mjThread_RunOnce( mjLF_Routine, server );
    // change to worker
    if ( !server->Routine ) {
        close( cfd );
        return NULL;
    }
    // create new conn 
    mjConnB conn = mjConnB_New( cfd );
    if ( !conn ) {
        MJLOG_ERR("create mjConnB error");
        close( cfd );
        return NULL;
    }
    server->Routine( conn );
    return NULL; 
}

void mjLF_Run( mjLF server )
{
    if ( !server ) return;
    while ( !server->shutdown ) {
        sleep(3);
        mjSig_ProcessQueue();
    }
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
    // init new pool
    server->tPool = mjThreadPool2_New( maxThread );
    if ( !server->tPool ) {
        MJLOG_ERR( "mjthreadpool create error" );
        free( server );
        return NULL;
    }
    // set server socket and routine
    server->sfd     = sfd;
    server->Routine = Routine;
    // add new worker 
    bool ret = mjThreadPool2_AddWork( server->tPool, mjLF_Routine, server );
    if ( !ret ) {
        MJLOG_ERR( "mjthreadpool addwork" );
        mjThreadPool2_Delete( server->tPool );
        free( server );
        return NULL;
    }
    // init signal
    mjSig_Init();
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
    // delete thread pool
    mjThreadPool2_Delete( server->tPool );
    free( server );
    return true;
}

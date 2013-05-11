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
    mjLF srv = ( mjLF ) arg;
    int cfd;
    // leader run this
    while ( 1 ) {
        cfd = mjSock_Accept( srv->sfd );
        if ( cfd < 0 ) {
            MJLOG_ERR("mjSock_Accept Error");
            continue;
        }
        break;
    }
    // choose a new leader
    int ret = mjThreadPool2_AddWork( srv->tPool, mjLF_Routine, srv );
    if ( !ret ) {
        ret = mjThread_RunOnce( mjLF_Routine, srv );
        if ( !ret ) {
            MJLOG_ERR( "Oops No Leader, Too Bad!!!" );
        }
    }
    // change to worker
    if ( !srv->Routine ) {
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
    mjConnB_SetServer( conn, srv );
    srv->Routine( conn );
    return NULL; 
}

bool mjLF_SetStop( mjLF srv, int value )
{
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }
    srv->stop = ( value == 0 ) ? 0 : 1;
    return true;
}

void mjLF_Run( mjLF srv )
{
    if ( !srv ) return;
    while ( !srv->stop ) {
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
    mjLF srv = ( mjLF ) calloc( 1, sizeof( struct mjLF ) );
    if ( !srv ) {
        MJLOG_ERR( "server create errror" );
        return NULL;
    }
    // init new pool
    srv->tPool = mjThreadPool2_New( maxThread );
    if ( !srv->tPool ) {
        MJLOG_ERR( "mjthreadpool create error" );
        free( srv );
        return NULL;
    }
    // set server socket and routine
    srv->sfd     = sfd;
    srv->Routine = Routine;
    // add new worker 
    bool ret = mjThreadPool2_AddWork( srv->tPool, mjLF_Routine, srv );
    if ( !ret ) {
        MJLOG_ERR( "mjthreadpool addwork" );
        mjThreadPool2_Delete( srv->tPool );
        free( srv );
        return NULL;
    }
    // init signal
    mjSig_Init();
    mjSig_Register( SIGPIPE, SIG_IGN );
    return srv;
}

/*
==========================================
mjLF_Delete
    Delete server
==========================================
*/
bool mjLF_Delete( mjLF srv )
{
    if ( !srv ) {
        MJLOG_ERR( "server is null" );
        return false;
    }
    // delete thread pool
    mjThreadPool2_Delete( srv->tPool );
    free( srv );
    return true;
}

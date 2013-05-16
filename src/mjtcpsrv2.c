#include "mjtcpsrv2.h"
#include "mjsock.h"

/*
===============================================================================
mjTcpSrv2_New
    alloc mjTcpSrv2 struct
===============================================================================
*/
/*
mjTcpSrv2 mjTcpSrv2_New( int sfd, mjProc Routine, int type ) {
    // alloc mjTcpSrv2 struct
    mjTcpSrv2 srv = ( mjTcpSrv2 ) calloc ( 1, sizeof( struct mjTcpSrv2 ) );    
    if ( !srv ) {
        MJLOG_ERR( "create server error" );
        goto failout1;
    }
    // check type
    if ( type != MJTCPSRV_STANDALONE && type != MJTCPSRV_INNER ) {
        MJLOG_ERR( "server type error" );
        goto failout2;
    }
    // set sfd nonblock
    mjSock_SetBlocking( srv->sfd, 0 );
    // set fields
    srv->sfd        = sfd;
    srv->type       = type;
    srv->Routine    = Routine;
    // set event Loop
    srv->ev = mjEV_New();
    if ( !srv->ev ) {
        MJLOG_ERR( "create ev error" );
        goto failout2;
    }
    // add read event
    if ( ( mjEV_Add( srv->ev, srv->sfd, MJEV_READABLE,
            mjTcpSrv2_AcceptRoutine, srv ) ) < 0 ) {
        MJLOG_ERR( "mjev add error" );
        goto failout3;
    }
    // set signal
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
*/

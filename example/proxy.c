#include <stdio.h>
#include "mjtcpsrv.h"
#include "mjsock.h"
#include "mjopt.h"
#include "mjconn.h"

void* On_Close( void* arg ) {
    mjConn conn = ( mjConn ) arg;
    mjConn_Delete( conn );
    return NULL;
}

void* On_Connect( void* arg ) {
    mjConn conn = ( mjConn ) arg;
    mjConn_WriteS( conn, "OK\r\n", NULL );
    mjConn_WriteS( conn, "OK2\r\n", NULL );
    mjConn_ReadUntil( conn, "\r\n\r\n", On_Close );
    return NULL;
}

int main() {
    int sfd = mjSock_TcpServer( 7879 );
    if ( !sfd ) {
        printf( "socket create error" );
        return 1;
    }

    mjTcpSrv srv = mjTcpSrv_New( sfd, On_Connect, MJTCPSRV_STANDALONE );
    if ( !srv ) {
        printf( "mjTcpSrv create error" );
        return 1;
    }
    
    mjTcpSrv_Run( srv );
    
    mjTcpSrv_Delete( srv );
    return 0;
}

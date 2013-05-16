#include <stdio.h>
#include "mjtcpsrvtp.h"
#include "mjsock.h"
#include "mjconnb.h"
#include "mjlog.h"

void* MyHandler( void* arg )
{
    mjConnB conn = ( mjConnB ) arg;

    mjStr data = mjStr_New();
    mjConnB_ReadUntil( conn, "\r\n\r\n", data );
    mjConnB_WriteS( conn, "OK I'm Here\r\n" );

    mjStr_Delete( data );
    mjConnB_Delete( conn );
    return NULL;
}

int main()
{
    int sfd = mjSock_TcpServer( 7879 );
    if ( sfd < 0 ) {
        printf( "mjSock_TcpServer error" );
        return 1;
    }

    mjTcpSrvTP srv = mjTcpSrvTP_New( sfd, 20 );
    if ( !srv ) {
        printf( "Error create server" );
        return 1;
    }

    mjTcpSrvTP_SetHandler( srv, MyHandler );
    mjTcpSrvTP_Run( srv );
    mjTcpSrvTP_Delete( srv );

    return 0;
}

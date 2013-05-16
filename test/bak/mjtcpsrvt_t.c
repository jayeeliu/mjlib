#include <stdio.h>
#include "mjsock.h"
#include "mjtcpsrvt.h"
#include "mjconnb.h"
#include "mjcomm.h"

void* myworker(void* arg)
{
    mjConnB conn = ( mjConnB )arg;
    mjConnB_SetTimeout( conn, 2000, 0 );

    mjStr data = mjStr_New();
    
    int ret = mjConnB_ReadUntil( conn, "\r\n\r\n", data );
    if ( ret <= 0 ) goto out;
    
    mjConnB_WriteS( conn, "HERE" );

out:
    mjStr_Delete( data );
    mjConnB_Delete( conn );
    return NULL;
}

int main()
{
    int sfd = mjSock_TcpServer( 7879 );
    if ( sfd < 0 ) {
        printf( "create socket error\n" );
        return 1;
    }

    mjTcpSrvT server = mjTcpSrvT_New( sfd, 20 );
    if ( !server ) {
        printf( "create server error\n" );
        return 1;
    }

    mjTcpSrvT_SetHandler( server, myworker );
    mjTcpSrvT_Run( server );
    mjTcpSrvT_Delete( server );

    return 0;
}

#include <stdio.h>
#include "mjsock.h"
#include "mjtcpsrvt.h"
#include "mjconnb.h"
#include "mjcomm.h"

void* myworker(void* arg)
{
    mjConnB conn = ( mjConnB )arg;
    mjConnB_SetTimeout( conn, 2000, 0 );

    mjstr data = mjstr_new();
    
    int ret = mjConnB_ReadUntil( conn, "\r\n\r\n", data );
    if ( ret <= 0 ) goto out;
    
    //int x = 6;
    //for(int i = 0; i < 1000000; ++i) {
    //    x = x * 13;
    //}
    mjConnB_WriteS( conn, "HERE" );

out:
    mjstr_delete( data );
    mjConnB_Delete( conn );
    return NULL;
}

int main()
{
    int sfd = mjsock_tcpserver( 7879 );
    if ( sfd < 0 ) {
        printf( "create socket error\n" );
        return 1;
    }

    mjTcpSrvT server = mjTcpSrvT_New( sfd, 100 );
    if ( !server ) {
        printf( "create server error\n" );
        return 1;
    }

    mjTcpSrvT_SetHandler( server, myworker );
    mjTcpSrvT_Run( server );
    mjTcpSrvT_Delete( server );

    return 0;
}

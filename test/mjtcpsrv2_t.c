#include <stdio.h>
#include "mjtcpsrv2.h"
#include "mjconn2.h"
#include "mjsock.h"

static void* on_close( void* arg )
{
    mjConn2 conn = ( mjConn2 ) arg;
    mjConn2_Delete( conn );
    return NULL;
}

static void* on_write( void* arg )
{
    mjConn2 conn = ( mjConn2 ) arg;
    long long sum=1;
    for(int i=1; i<100000; i++) {
        sum *= i;
    }
    mjConn2_WriteS( conn, "Final Server Ready!!!\r\n", on_close );
    return NULL;
}

static void* Routine( void* arg )
{
    mjConn2 conn = ( mjConn2 ) arg;
    mjConn2_ReadUntil( conn, "\r\n\r\n", on_write);
    return NULL;
}

int main()
{
    int sfd = mjSock_TcpServer(7879);
    if ( sfd < 0 ) {
        printf("socket create error\n" );
        return -1;
    }

    mjServer server = mjServer_New( sfd, Routine );
    if ( !server ) {
        printf( "mjServer create error\n" );
        return -1;
    }
    mjServer_Run( server );
    mjServer_Delete( server );
    return 0;
}

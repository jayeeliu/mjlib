#include <stdio.h>
#include "mjtcpsrv2.h"
#include "mjconn.h"
#include "mjsock.h"

static void* on_close( void* arg )
{
    mjConn conn = ( mjConn ) arg;
    mjConn_Delete( conn );
    return NULL;
}

static void* on_write( void* arg )
{
    mjConn conn = ( mjConn ) arg;
    mjConn_WriteS( conn, "Final Server Ready!!!\r\n", on_close );
    return NULL;
}

static void* Routine( void* arg )
{
    mjConn conn = ( mjConn ) arg;
    mjConn_ReadUntil( conn, "\r\n\r\n", on_write);
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

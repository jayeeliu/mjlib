#include <stdio.h>
#include "mjmainsrv.h"
#include "mjconn.h"
#include "mjsock.h"

static void* on_close( void* arg )
{
    mjConn conn = ( mjConn ) arg;
    mjConn_Delete( conn );
    return NULL;
}

static void* calRoutine( void* arg ) {
    return NULL;
}

static void* on_write( void* arg )
{
    mjConn conn = ( mjConn ) arg;
    mjConn_WriteS( conn, "Final Server Ready!!!\r\n", NULL );
    mjMainSrv_Async( ( ( mjTcpSrv2 )conn->server )->mainServer, calRoutine, NULL, 
            conn->ev, on_close, conn );
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

    mjMainSrv server = mjMainSrv_New( sfd, Routine, 10 );
    if ( !server ) {
        printf( "mjMainSrv create error\n" );
        return -1;
    }
    mjMainSrv_Run( server );
    mjMainSrv_Delete( server );
    return 0;
}

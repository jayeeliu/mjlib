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

static void* calRoutine( void* arg ) {
    return NULL;
}

static void* on_write( void* arg )
{
    mjConn2 conn = ( mjConn2 ) arg;
    mjConn2_WriteS( conn, "Final Server Ready!!!\r\n", NULL );
    mjMainServer_Async( ( ( mjTcpSrv2 )conn->server )->mainServer, calRoutine, NULL, 
            conn->ev, on_close, conn );
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

    mjMainServer server = mjMainServer_New( sfd, Routine, 10 );
    if ( !server ) {
        printf( "mjMainServer create error\n" );
        return -1;
    }
    mjMainServer_Run( server );
    mjMainServer_Delete( server );
    return 0;
}

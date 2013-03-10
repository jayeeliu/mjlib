#include <stdio.h>
#include <unistd.h>
#include "mjtcpsrv.h"
#include "mjsock.h"
#include "mjopt.h"
#include "mjconn.h"

static int port;

void Option_Init()
{
    mjOpt_Define( NULL, "port", MJOPT_INT, &port, "7879", "p", 1, "set port" );
}

void on_close( void* arg )
{
    mjconn conn = ( mjconn )arg;
    mjConn_Delete( conn );
}

void on_read( void* arg )
{
    mjconn conn = ( mjconn )arg;
    mjStrList strList = mjStrList_New();
    mjstr_split( conn->data, " ", strList );
    
    
    mjConn_Write( conn, mjStrList_Get( strList, 0 ), on_close );

    mjStrList_Delete( strList );
}

void Msg_Handler( void* arg )
{
    mjconn conn = ( mjconn )arg;
    mjConn_ReadUntil( conn, "\r\n", on_read );
}

int main( int argc, char* argv[] )
{
    Option_Init();

    mjOpt_ParseCmd( argc, argv );
    int sfd = mjsock_tcpserver( port );
    if ( sfd < 0 ) {
        printf( "socket create error" );
        return 1;
    }

    mjTcpSrv srv = mjTcpSrv_New( sfd );
    if ( !srv ) {
        printf( "server create error" );
        close( sfd );
        return 1;
    }

    mjTcpSrv_SetHandler( srv, Msg_Handler );
    mjTcpSrv_Run( srv );
    mjTcpSrv_Delete( srv );
    return 0;
}

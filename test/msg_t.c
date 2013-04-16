#include <stdio.h>
#include <string.h>
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

void* on_close( void* arg )
{
    mjConn conn = ( mjConn )arg;
    mjConn_Delete( conn );
    return NULL;
}

void* on_read( void* arg )
{
    mjConn conn = ( mjConn )arg;
    mjStrList strList = mjStrList_New();
    mjStr_Split( conn->data, " ", strList );
    
    mjStr comm = mjStrList_Get( strList, 0 );    
    if ( !strcmp( comm->data, "get" ))  {
        mjConn_WriteS( conn, "get command run\n", on_close );
    } else if ( !strcmp( comm->data, "put" ) ) {
        mjConn_WriteS( conn, "put command run\n", on_close );
    } else {
        mjConn_WriteS( conn, "other command run\n", on_close );
    }

    mjStrList_Delete( strList );
    return NULL;
}

void* Msg_Handler( void* arg )
{
    mjConn conn = ( mjConn )arg;
    mjConn_ReadUntil( conn, "\r\n", on_read );
    return NULL;
}

int main( int argc, char* argv[] )
{
    Option_Init();

    mjOpt_ParseCmd( argc, argv );
    int sfd = mjSock_TcpServer( port );
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

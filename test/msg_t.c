#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "mjtcpsrv.h"
#include "mjsock.h"
#include "mjopt.h"
#include "mjconn2.h"

void* on_close( void* arg )
{
    mjConn2 conn = ( mjConn2 )arg;
    mjConn2_Delete( conn );
    return NULL;
}

void* on_read( void* arg )
{
    mjConn2 conn = ( mjConn2 )arg;
    mjStrList strList = mjStrList_New();
    mjStr_Split( conn->data, " ", strList );
    
    mjStr comm = mjStrList_Get( strList, 0 );    
    if ( !strcmp( comm->data, "get" ))  {
        mjConn2_WriteS( conn, "get command run\n", on_close );
    } else if ( !strcmp( comm->data, "put" ) ) {
        mjConn2_WriteS( conn, "put command run\n", on_close );
    } else {
        mjConn2_WriteS( conn, "other command run\n", on_close );
    }

    mjStrList_Delete( strList );
    return NULL;
}

void* Msg_Handler( void* arg )
{
    mjConn2 conn = ( mjConn2 )arg;
    mjConn2_ReadUntil( conn, "\r\n", on_read );
    return NULL;
}

int main( int argc, char* argv[] )
{
    int sfd = mjSock_TcpServer( 7879 );
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

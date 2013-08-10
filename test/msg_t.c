#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "mjtcpsrv.h"
#include "mjsock.h"
#include "mjopt.h"
#include "mjconn.h"

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
    int sfd = mjSock_TcpServer( 7879 );
    if ( sfd < 0 ) {
        printf( "socket create error" );
        return 1;
    }

    mjtcpsrv srv = mjtcpsrv_new( sfd, Msg_Handler, MJTCPSRV_STANDALONE );
    if ( !srv ) {
        printf( "server create error" );
        close( sfd );
        return 1;
    }

    mjtcpsrv_run( srv );
    mjtcpsrv_delete( srv );
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include "mjsock.h"
#include "mjtcpsrv.h"
#include "mjconn.h"
#include "mjlog.h"

struct session {
    mjConn  serverConn;
    mjConn  clientConn1;
    mjConn  clientConn2;
    int     connected;
};
typedef struct session* session;

void* On_Connect( void* arg )
{
    mjConn conn = ( mjConn ) arg;
    session sess = conn->private;
    sess->connected++;

    if ( sess->connected == 2 ) {
        MJLOG_ERR( "Connect OK" );
        mjConn_Delete( sess->serverConn );
        mjConn_Delete( sess->clientConn1 );
        mjConn_Delete( sess->clientConn2 );
    }
    return NULL;
}

void* RouterHandler( void* arg )
{
    mjConn conn = ( mjConn ) arg;
    
    session sess = malloc( sizeof( struct session ) );
    if ( !sess ) {
        MJLOG_ERR( "malloc error" );
        mjConn_Delete( conn );
        return NULL;
    }
    sess->serverConn    = conn;
    sess->connected     = 0;
    mjConn_SetPrivate( conn, sess, NULL );

    int clientSock1 = mjSock_TcpSocket();
    int clientSock2 = mjSock_TcpSocket();

    sess->clientConn1 = mjConn_New( conn->ev, clientSock1 );
    sess->clientConn2 = mjConn_New( conn->ev, clientSock2 );
    mjConn_SetPrivate( sess->clientConn1, sess, NULL );
    mjConn_SetPrivate( sess->clientConn2, sess, NULL );

    mjConn_Connect( sess->clientConn1, "123.126.42.251", 80, On_Connect );
    mjConn_Connect( sess->clientConn2, "123.126.42.251", 80, On_Connect );

    return NULL;
}

int main()
{
    int sock = mjSock_TcpServer( 7879 );
    if ( sock < 0 ) {
        printf( "mjSock_TcpServer Error\n" );
        return 1;
    }

    mjTcpSrv server = mjTcpSrv_New( sock, RouterHandler, MJTCPSRV_STANDALONE );

    mjTcpSrv_Run( server );
    mjTcpSrv_Delete( server );
    return 0;
}

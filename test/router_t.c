#include <stdio.h>
#include <stdlib.h>
#include "mjsock.h"
#include "mjtcpsrv.h"
#include "mjconn2.h"
#include "mjlog.h"

struct session {
    mjConn2  serverConn;
    mjConn2  clientConn1;
    mjConn2  clientConn2;
    int     connected;
};
typedef struct session* session;

void* On_Connect( void* arg )
{
    mjConn2 conn = ( mjConn2 ) arg;
    session sess = conn->private;
    sess->connected++;

    if ( sess->connected == 2 ) {
        MJLOG_ERR( "Connect OK" );
        mjConn2_Delete( sess->serverConn );
        mjConn2_Delete( sess->clientConn1 );
        mjConn2_Delete( sess->clientConn2 );
    }
    return NULL;
}

void* RouterHandler( void* arg )
{
    mjConn2 conn = ( mjConn2 ) arg;
    
    session sess = malloc( sizeof( struct session ) );
    if ( !sess ) {
        MJLOG_ERR( "malloc error" );
        mjConn2_Delete( conn );
        return NULL;
    }
    sess->serverConn    = conn;
    sess->connected     = 0;
    mjConn2_SetPrivate( conn, sess, NULL );

    int clientSock1 = mjSock_TcpSocket();
    int clientSock2 = mjSock_TcpSocket();

    sess->clientConn1 = mjConn2_New( conn->ev, clientSock1 );
    sess->clientConn2 = mjConn2_New( conn->ev, clientSock2 );
    mjConn2_SetPrivate( sess->clientConn1, sess, NULL );
    mjConn2_SetPrivate( sess->clientConn2, sess, NULL );

    mjConn2_Connect( sess->clientConn1, "123.126.42.251", 80, On_Connect );
    mjConn2_Connect( sess->clientConn2, "123.126.42.251", 80, On_Connect );

    return NULL;
}

int main()
{
    int sock = mjSock_TcpServer( 7879 );
    if ( sock < 0 ) {
        printf( "mjSock_TcpServer Error\n" );
        return 1;
    }

    mjTcpSrv server = mjTcpSrv_New( sock );

    mjTcpSrv_SetHandler( server, RouterHandler );
    mjTcpSrv_Run( server );
    mjTcpSrv_Delete( server );
    return 0;
}

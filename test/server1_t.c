#include <stdio.h>
#include <stdlib.h>
#include "mjtcpsrv.h"
#include "mjsock.h"
#include "mjopt.h"
#include "mjconn.h"
#include "mjlog.h"

static int port;

struct connPrivate {
    mjConn  conn1;
    mjConn  conn2;
};
typedef struct connPrivate* connPrivate;

void Option_Init()
{
    mjOpt_Define( NULL, "port", MJOPT_INT, &port, "7879", "p", 1, "set port" );
}

void On_Close( void* arg )
{
    mjConn conn = ( mjConn ) arg;
    mjConn_Delete( conn );
}

void Conn1_OK( void* arg )
{
    mjConn conn1 = ( mjConn ) arg;
    mjConn conn = conn1->private;

    mjConn_WriteS( conn, "Conn1 OK", On_Close );
}

void Client_Clean( void* arg )
{
    mjConn conn = ( mjConn ) arg;
    mjConn_Delete( conn );
}

void Handler( void* arg )
{
    mjConn conn = ( mjConn ) arg;
    connPrivate connP = ( connPrivate ) calloc ( 1, 
                    sizeof( struct connPrivate ) );
    if ( !connP ) {
        MJLOG_ERR( "alloc connPrivate error" );
        mjConn_Delete( conn );
        return;
    }

    mjConn_SetPrivate( conn, connP, NULL );
    
    int cfd1 = mjSock_TcpSocket();
    int cfd2 = mjSock_TcpSocket();
    connP->conn1 = mjConn_New( conn->ev, cfd1 );
    connP->conn1 = mjConn_New( conn->ev, cfd2 );

    mjConn_SetPrivate( connP->conn1, conn, Client_Clean );
    mjConn_SetConnectTimeout( connP->conn1, 1000 );
    mjConn_Connect( connP->conn1, "127.0.0.1", 3306, Conn1_OK);
}

int main()
{
    Option_Init();

    int sfd = mjSock_TcpServer(port);
    if ( sfd < 0 ) {
        printf( "mjSock_TcpServer error" );
        return 1;
    }

    mjTcpSrv server = mjTcpSrv_New( sfd );
    if ( !server ) {
        printf( "mjTcpSrv_New error" );
        return 1;
    }

    mjTcpSrv_SetHandler( server, Handler );
    mjTcpSrv_Run( server );
    mjTcpSrv_Delete( server );
    return 0;
}
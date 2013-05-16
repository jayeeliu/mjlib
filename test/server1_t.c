#include <stdio.h>
#include <stdlib.h>
#include "mjtcpsrv.h"
#include "mjsock.h"
#include "mjopt.h"
#include "mjconn2.h"
#include "mjlog.h"

struct connPrivate {
    mjConn2  conn1;
    mjConn2  conn2;
};
typedef struct connPrivate* connPrivate;

void* On_Close( void* arg )
{
    mjConn2 conn = ( mjConn2 ) arg;
    mjConn2_Delete( conn );
    return NULL;
}

void* Conn1_OK( void* arg )
{
    mjConn2 conn1 = ( mjConn2 ) arg;
    mjConn2 conn = conn1->private;

    mjConn2_WriteS( conn, "Conn1 OK", On_Close );
    return NULL;
}

void* Client_Clean( void* arg )
{
    mjConn2 conn = ( mjConn2 ) arg;
    mjConn2_Delete( conn );
    return NULL;
}

void* Handler( void* arg )
{
    mjConn2 conn = ( mjConn2 ) arg;
    connPrivate connP = ( connPrivate ) calloc ( 1, 
                    sizeof( struct connPrivate ) );
    if ( !connP ) {
        MJLOG_ERR( "alloc connPrivate error" );
        mjConn2_Delete( conn );
        return NULL;
    }

    mjConn2_SetPrivate( conn, connP, NULL );
    
    int cfd1 = mjSock_TcpSocket();
    int cfd2 = mjSock_TcpSocket();
    connP->conn1 = mjConn2_New( conn->ev, cfd1 );
    connP->conn1 = mjConn2_New( conn->ev, cfd2 );

    mjConn2_SetPrivate( connP->conn1, conn, Client_Clean );
    mjConn2_SetConnectTimeout( connP->conn1, 1000 );
    mjConn2_Connect( connP->conn1, "127.0.0.1", 3306, Conn1_OK);
    return NULL;
}

int main()
{
    int sfd = mjSock_TcpServer(7879);
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "mjsock.h"
#include "mjconn.h"
#include "mjtcpsrv.h"
#include "mjcomm.h"
#include "mjlog.h"

void on_close(void *data)
{
    mjConn conn = (mjConn)data;
    mjConn_Delete(conn);
}

void on_write3(void *data)
{
    mjConn conn = (mjConn)data;
    mjConn_Flush(conn, on_close);
}

void *threadroutine(void *data)
{
    mjConn conn = (mjConn) data;
    mjStr_CatS(conn->wbuf, "data from thread\n");
    return NULL;
}

void on_write2(void *data)
{
    mjConn conn = (mjConn) data;
    mjConn_RunAsync(conn, threadroutine, on_write3);
}

void on_write1(void *data)
{
    mjConn conn = (mjConn)data;
//    int x = 6;
//    for(int i = 0; i < 100000; ++i) {
//        x = x * 13;
//    }
    mjConn_WriteS(conn, "OK, TCPSERVER READY!!!\n", on_close);
}
void myhandler(void *data)
{
    mjConn conn = (mjConn)data;
    mjConn_ReadUntil(conn, "\r\n\r\n", on_write1);
}

void On_ReadResponse( void* args )
{
    mjConn clientConn = ( mjConn ) args;
    printf("%s\n", clientConn->data->data);
    MJLOG_ERR( " OK RESPONSE " );
    mjConn_Delete( clientConn->private );    
}

void On_WriteHeader( void* args )
{
    mjConn clientConn = ( mjConn ) args;
    mjConn_ReadUntil( clientConn, "\r\n", On_ReadResponse );
}

void On_Connect( void* args )
{
    mjConn clientConn = ( mjConn ) args;
    mjConn_WriteS( clientConn, "GET / HTTP/1.1\r\n\r\n", On_WriteHeader );
}

void FreeClient( void* args )
{
    mjConn clientConn = ( mjConn ) args;
    mjConn_Delete( clientConn );
}

void proxyhandler( void* args ) 
{
    mjConn conn = ( mjConn ) args;
    int cfd = mjSock_TcpSocket();
    mjConn clientConn = mjConn_New( conn->ev, cfd );
    clientConn->private = conn;
    mjConn_SetPrivate( conn, clientConn, FreeClient );
    mjConn_Connect( clientConn, "202.108.33.60", 80, On_Connect ); 
}

int main()
{
    int sfd = mjSock_TcpServer(7879);
    if (sfd < 0) {
        printf("Error create server socket\n");
        return 1;
    }

    mjTcpSrv server = mjTcpSrv_New(sfd); 
    if ( !server ) {
        printf("Error create tcpserver\n");
        return 1;
    }

    mjTcpSrv_SetHandler(server, myhandler);
    mjTcpSrv_Run(server);

    mjTcpSrv_Delete(server); 
    return 0;
}

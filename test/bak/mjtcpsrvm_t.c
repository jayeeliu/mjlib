#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "mjsock.h"
#include "mjconn.h"
#include "mjtcpsrv.h"
#include "mjcomm.h"

void* on_close(void *data)
{
    mjConn conn = (mjConn)data;
    mjConn_Delete(conn);
    return NULL;
}

void* on_write3(void *data)
{
    mjConn conn = (mjConn)data;
    mjConn_Flush(conn, on_close);
    return NULL;
}

void* threadroutine(void *data)
{
    mjConn conn = (mjConn) data;
    mjStr_CatS(conn->wbuf, "data from thread\n");
    return NULL;
}

void* on_write2(void *data)
{
    mjConn conn = (mjConn) data;
    mjConn_RunAsync(conn, threadroutine, on_write3);
    return NULL;
}

void* on_write1(void *data)
{
    mjConn conn = (mjConn)data;
    mjConn_WriteS(conn, "OK, TCPSERVER READY!!!\n", on_close);
    return NULL;
}
void* myhandler(void *data)
{
    mjConn conn = (mjConn)data;
    mjConn_ReadUntil(conn, "\r\n\r\n", on_write1);
    return NULL;
}

int main()
{
    int sfd = mjSock_TcpServer(7879);
    if (sfd < 0) {
        printf("Error create server socket\n");
        return 1;
    }

    ProcessSpawn( 4 );
    mjTcpSrv server = mjTcpSrv_New(sfd); 
    if (!server) {
        printf("Error create tcpserver\n");
        return 1;
    }
    mjTcpSrv_SetHandler(server, myhandler);
    mjTcpSrv_Run(server);

    mjTcpSrv_Delete(server); 
    return 0;
}

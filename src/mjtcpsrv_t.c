#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "mjsock.h"
#include "mjconn.h"
#include "mjtcpsrv.h"
#include "mjcomm.h"

void on_close(void *data)
{
    mjconn conn = (mjconn)data;
    mjConn_Delete(conn);
}

void on_write3(void *data)
{
    mjconn conn = (mjconn)data;
    mjConn_Flush(conn, on_close);
}

void *threadroutine(void *data)
{
    mjconn conn = (mjconn) data;
    mjstr_cats(conn->wbuf, "data from thread\n");
    return NULL;
}

void on_write2(void *data)
{
    mjconn conn = (mjconn) data;
    mjConn_RunAsync(conn, threadroutine, on_write3);
}

void on_write1(void *data)
{
    mjconn conn = (mjconn)data;
    int x = 6;
    for(int i = 0; i < 100000; ++i) {
        x = x * 13;
    }
    mjConn_WriteS(conn, "OK, TCPSERVER READY!!!\n", on_close);
}
void myhandler(void *data)
{
    mjconn conn = (mjconn)data;
    mjConn_ReadUntil(conn, "\r\n\r\n", on_write1);
}

int main()
{
    int sfd = mjsock_tcpserver(7879);
    if (sfd < 0) {
        printf("Error create server socket\n");
        return 1;
    }

//    ProcessSpawn( 2 );
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

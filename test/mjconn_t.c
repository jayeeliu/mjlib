#include <stdio.h>
#include <unistd.h>
#include "mjtcpsrv.h"
#include "mjsock.h"
#include "mjconn.h"
#include "mjcomm.h"
#include "mjpool.h"

void on_close(void *arg)
{
    mjConn conn = (mjConn)arg;
    mjConn_Delete(conn);
}

void* thread_run(void *arg)
{
    sleep(1);
    return NULL;
}

void thread_finish(void *arg)
{
    mjConn conn = (mjConn)arg;
    mjConn_WriteS(conn, "thread finish", on_close);
}

void finish_write(void *arg)
{
    mjConn conn = (mjConn)arg;
    mjConn_RunAsync(conn, thread_run, thread_finish);
}

extern void handle_stream(void *arg);

void handle_request(void *arg)
{
    mjConn conn = (mjConn)arg;
    mjConn_Write(conn, conn->data, handle_stream); 
}

void handle_stream(void *arg)
{
    mjConn conn = (mjConn)arg;
    mjConn_Read(conn, handle_request); 
}

int main()
{
    int sfd = mjSock_TcpServer(7879);
    if (sfd < 0) {
        printf("Error to create socket");
        return 1;
    }

    mjTcpSrv server = mjTcpSrv_New(sfd);
    if (server == NULL) {
        printf("can't create server\n");
        close(sfd);
        return 1;
    }

    mjTcpSrv_SetHandler( server, handle_stream );
    mjTcpSrv_Run(server);

    mjTcpSrv_Delete( server );

    return 0;
}

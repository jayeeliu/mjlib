#include <stdio.h>
#include "mjmainsrv.h"
#include "mjconn.h"
#include "mjsock.h"

static void* on_close(void* arg)
{
    mjConn conn = (mjConn) arg;
    mjConn_Delete(conn);
    return NULL;
}

static void* calRoutine(void* arg) {
    return NULL;
}

static void* on_write(void* arg)
{
    mjConn conn = (mjConn) arg;
    mjConn_WriteS(conn, "Final Server Ready!!!\r\n", NULL);
    mjmainsrv_async(conn->server, calRoutine, NULL, on_close, conn);
    return NULL;
}

static void* Routine(void* arg)
{
    mjConn conn = (mjConn) arg;
    mjConn_ReadUntil(conn, "\r\n\r\n", on_write);
    return NULL;
}

int main()
{
    int sfd = mjsock_tcp_server(7879);
    if (sfd < 0) {
        printf("socket create error\n");
        return -1;
    }

    mjmainsrv server = mjmainsrv_new(sfd, Routine, NULL, NULL, 10);
    if (!server) {
        printf("mjmainsrv create error\n");
        return -1;
    }
    mjmainsrv_run(server);
    mjmainsrv_delete(server);
    return 0;
}

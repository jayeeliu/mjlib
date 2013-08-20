#include <stdio.h>
#include "mjmainsrv.h"
#include "mjconn.h"
#include "mjsock.h"
#include "mjsql.h"

static void* on_close(void* arg)
{
    mjconn conn = (mjconn) arg;
    mjconn_delete(conn);
    return NULL;
}

static void* calRoutine(void* arg) {
    return NULL;
}

static void* on_write(void* arg)
{
    mjconn conn = (mjconn) arg;
    mjconn_writes(conn, "Final Server Ready!!!\r\n", NULL);
		mjtcpsrv server = (mjtcpsrv) mjconn_get_obj(conn, "server");
    mjmainsrv_async(server, calRoutine, NULL, on_close, conn);
    return NULL;
}

static void* Routine(void* arg)
{
    mjconn conn = (mjconn) arg;
    mjconn_readuntil(conn, "\r\n\r\n", on_write);
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

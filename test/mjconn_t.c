#include <stdio.h>
#include <sys/socket.h>
#include "mjconn.h"
#include "mjev.h"
#include "mjlog.h"

static int stop = 0;

void* conn_close(void *arg)
{
    mjConn conn = (mjConn)arg;
    MJLOG_ERR( "write ok close" );
    mjConn_Delete(conn);
    stop = 1;
    return NULL;
}

void* conn_write(void *arg)
{
    mjConn conn = (mjConn)arg;
    MJLOG_ERR( "connect OK begin write" );
    mjConn_WriteS(conn, "test\r\n\r\n", conn_close);
    return NULL;
}

int main()
{
    mjev ev = mjev_new();
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    
    mjConn conn = mjConn_New(ev, cfd);
    mjConn_SetConnectTimeout(conn, 2000);
    mjConn_Connect(conn, "127.0.0.1", 7879, conn_write);

    while (!stop) {
        mjev_run(ev);
    }
    mjev_delete(ev);

    return 0;
}

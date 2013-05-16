#include <stdio.h>
#include <sys/socket.h>
#include "mjconn2.h"
#include "mjev.h"
#include "mjlog.h"

static int stop = 0;

void* conn_close(void *arg)
{
    mjConn2 conn = (mjConn2)arg;
    MJLOG_ERR( "write ok close" );
    mjConn2_Delete(conn);
    stop = 1;
    return NULL;
}

void* conn_write(void *arg)
{
    mjConn2 conn = (mjConn2)arg;
    MJLOG_ERR( "connect OK begin write" );
    mjConn2_WriteS(conn, "test\r\n\r\n", conn_close);
    return NULL;
}

int main()
{
    mjEV ev = mjEV_New();
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    
    mjConn2 conn = mjConn2_New(ev, cfd);
    mjConn2_SetConnectTimeout(conn, 2000);
    mjConn2_Connect(conn, "127.0.0.1", 7879, conn_write);

    while (!stop) {
        mjEV_Run(ev);
    }
    mjEV_Delete(ev);

    return 0;
}

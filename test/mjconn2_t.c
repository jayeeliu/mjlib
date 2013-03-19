#include <stdio.h>
#include <sys/socket.h>
#include "mjconn.h"
#include "mjev.h"

static int stop = 0;

void conn_close(void *arg)
{
    mjConn conn = (mjConn)arg;
    mjConn_Delete(conn);
    stop = 1;
}

void conn_write(void *arg)
{
    mjConn conn = (mjConn)arg;
    mjConn_WriteS(conn, "test\r\n\r\n", conn_close);
}

int main()
{
    mjev ev = mjEV_New();
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    
    mjConn conn = mjConn_New( ev, cfd);
    mjConn_SetConnectTimeout(conn, 2000);
    mjConn_Connect(conn, "12.1.1.12", 7879, conn_write);

    while (!stop) {
        mjEV_Run(ev);
    }
    mjEV_Delete(ev);

    return 0;
}

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "mjsock.h"
#include "mjconn.h"
#include "mjtcpsrv.h"
#include "mjcomm.h"
#include "mjlog.h"

//static int count = 0;

void* on_close(void *data)
{
    mjconn conn = (mjconn)data;
//    count++;
//    if (count > 100000) {
//      mjtcpsrv server = (mjtcpsrv) mjconn_get_obj(conn, "server");
//      mjtcpsrv_set_stop(server, true);
//    }
    mjconn_delete(conn);
    return NULL;
}

void* on_write1(void *data) {
  mjconn conn = (mjconn)data;
  mjconn_writes(conn, "OK, TCPSERVER READY!!!\n", on_close);
  return NULL;
}

void* myhandler(void *data)
{
    mjconn conn = (mjconn)data;
    mjconn_readuntil(conn, "\r\n\r\n", on_write1);
    return NULL;
}

int main()
{
    int sfd = mjsock_tcp_server(7879);
    if (sfd < 0) {
        printf("Error create server socket\n");
        return 1;
    }
    mjtcpsrv server = mjtcpsrv_new(sfd, myhandler, NULL, NULL, MJTCPSRV_STANDALONE); 
    if ( !server ) {
        printf("Error create tcpserver\n");
        return 1;
    }
    mjtcpsrv_run(server);
    mjtcpsrv_delete(server); 
    return 0;
}

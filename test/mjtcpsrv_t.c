#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "mjsock.h"
#include "mjconn.h"
#include "mjtcpsrv.h"
#include "mjcomm.h"
#include "mjlog.h"

void* on_close(void *arg) {
  mjconn conn = arg;
  mjconn_delete(conn);
  return NULL;
}

void* on_write(void *arg) {
  mjconn conn = arg;
  if (conn->_error || conn->_closed || conn->_timeout) {
    mjconn_delete(conn);
    return NULL;
  }
  mjconn_writes(conn, "OK, TCPSERVER READY!!!\n", on_close);
  return NULL;
}

void* myhandler(void *arg) {
  mjconn conn = arg;
  mjconn_readuntil(conn, "\r\n\r\n", on_write);
  return NULL;
}

int main() {
  int sfd = mjsock_tcp_server(7879);
  if (sfd < 0) {
    printf("Error create server socket\n");
    return 1;
  }
  mjtcpsrv server = mjtcpsrv_new(sfd, MJTCPSRV_STANDALONE); 
  if (!server) {
    printf("Error create tcpserver\n");
    return 1;
  }
  //mjtcpsrv_set_mutex(server, mutex);
  mjtcpsrv_set_task(server, myhandler);
  mjtcpsrv_run(server);
  mjtcpsrv_delete(server); 
  return 0;
}

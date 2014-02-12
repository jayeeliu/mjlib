#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "mjsock.h"
#include "mjconn.h"
#include "mjsrv.h"
#include "mjcomm.h"
#include "mjlog.h"

static void* on_close(void *arg) {
  mjconn conn = arg;
  mjconn_delete(conn);
  return NULL;
}

static void* testRoutine(void *arg) {
  return NULL;
}

static void* finishRoutine(void *arg) {
  mjconn conn = arg;
  mjconn_writes(conn, "OK, TCPSERVER READY!!!\n", on_close);
  return NULL;
}

static void* on_write(void *arg) {
  mjconn conn = arg;
  if (conn->_error || conn->_closed || conn->_timeout) {
    mjconn_delete(conn);
    return NULL;
  }
  mjsrv srv = mjconn_get_obj(conn, "server");
  mjsrv_async(srv, testRoutine, NULL, finishRoutine, conn);
  return NULL;
}

static void* myhandler(void *arg) {
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
  mjsrv server = mjsrv_new(sfd); 
  if (!server) {
    printf("Error create tcpserver\n");
    return 1;
  }
  mjsrv_set_tpool(server, 4);
  mjsrv_set_task(server, myhandler);
  mjsrv_run(server);
  mjsrv_delete(server); 
  return 0;
}

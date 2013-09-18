#include <stdio.h>
#include "mjmainsrv.h"
#include "mjconn.h"
#include "mjsock.h"
#include "mjsql.h"

static void* on_close(void* arg) {
  mjconn conn = (mjconn) arg;
  mjconn_delete(conn);
  return NULL;
}

static void* on_fin(void* arg) {
  mjconn conn = (mjconn) arg;
  mjconn_flush(conn, on_close);
  return NULL; 
}

static void* calRoutine(void* arg) {
  mjconn conn = (mjconn) arg;
  mjconn_buf_writes(conn, "cal begin\r\n");
  long long x = 1;
  for (int i = 1; i < 100000000; i++) {
    x = x + i;
  }
  char buf[1024];
  sprintf(buf, "result is: %lld\r\n", x);
  mjconn_buf_writes(conn, buf);
  mjconn_buf_writes(conn, "cal end\r\n");
  return NULL;
}

static void* on_cal(void* arg) {
  mjconn conn = (mjconn) arg;
  if (conn->_timeout) {
    mjconn_delete(conn);
    return NULL;
  }
  mjtcpsrv server = (mjtcpsrv) mjconn_get_obj(conn, "server");
  mjmainsrv_async(server, calRoutine, conn, on_fin, conn);
  return NULL;
}

static void* Routine(void* arg) {
  mjconn conn = (mjconn) arg;
  mjconn_readuntil(conn, "\r\n\r\n", on_cal);
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

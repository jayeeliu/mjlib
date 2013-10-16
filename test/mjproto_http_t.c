#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include "mjconn.h"
#include "mjproto_http.h"
#include "mjtcpsrv.h"
#include "mjsock.h"
#include "mjlog.h"

static int count;
static void* on_finish(void *arg) {
  mjconn conn = (mjconn) arg;
  //count++;
  if (count > 10) {
    mjtcpsrv srv = mjconn_get_obj(conn, "server");
    mjtcpsrv_set_stop(srv, true);
  }
  mjconn_delete(conn);
  return NULL;
}

static void* main0(void *arg) {
  mjconn conn = (mjconn)arg;
  mjhttpdata hdata = mjconn_get_obj(conn, "httpdata");
  mjhttprsp_set_status(hdata->rsp, 200);
  mjstr header = mjhttprsp_header_to_str(hdata->rsp);
  mjconn_buf_write(conn, header);
  mjconn_buf_writes(conn, "\r\n");
  mjconn_writes(conn, "main0 is here", on_finish);
  mjstr_delete(header);
  return NULL;
}

struct mjhttpurl urls[] = {
  {"^/$", main0, NULL},
  {NULL, on_finish, NULL},
};

int main() {
  // create new tcpsrver socket
  int sfd = mjsock_tcp_server(7879);
  if (sfd < 0) {
    printf("Error create server socket\n");
    return 1;
  }

  mjtcpsrv srv = mjtcpsrv_new(sfd, http_mjtcpsrv_routine, 
      http_mjtcpsrv_init, urls, MJTCPSRV_STANDALONE);
  if (!srv) {
    printf("Error create tcpserver\n");
    return 1;
  }
  mjtcpsrv_run(srv);
  mjtcpsrv_delete(srv);
  return 0;
}

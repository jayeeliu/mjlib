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
  mjhttprsp_set_strs(hdata->rsp, "OK This is the test string");
  mjstr content = mjhttprsp_to_str(hdata->rsp);
  mjconn_write(conn, content, on_finish);
  mjstr_delete(content);
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
  mjtcpsrv srv = mjtcpsrv_new(sfd, MJTCPSRV_STANDALONE);
  if (!srv) {
    printf("Error create tcpserver\n");
    return 1;
  }
	mjtcpsrv_set_init(srv, http_mjtcpsrv_init, urls);
	mjtcpsrv_set_routine(srv, http_mjtcpsrv_routine);
  mjtcpsrv_run(srv);
  mjtcpsrv_delete(srv);
  return 0;
}

#include "mjproto_http.h"
#include "mjconnb.h"
#include "mjsock.h"
#include "mjlf.h"

int count = 0;

void* def(void* arg) {
  mjconnb conn = (mjconnb) arg;
  mjconnb_writes(conn, "Page No Found!");
  return NULL;
}

void* main0(void* arg) {
  mjconnb conn = (mjconnb) arg;
  mjconnb_writes(conn, "main0");
//  count++;
//  if (count > 1000) mjlf_set_stop(conn->server, true);
  return NULL;
}

void* main1(void* arg) {
  mjconnb conn = (mjconnb) arg;
  mjconnb_writes(conn, "main1");
  return NULL;
}

struct mjhttpurl urls[] = {
  {"^/1/$", main1, NULL},
  {"^/$", main0, NULL},
  {NULL, def, NULL}
};

int main() {
  int srv_sock = mjsock_tcp_server(7879);
  mjlf srv = mjlf_new(srv_sock, http_mjlf_routine, 2, http_mjlf_init, urls,
      NULL, NULL);
  mjlf_run(srv);
  mjlf_delete(srv);
  return 0;
}

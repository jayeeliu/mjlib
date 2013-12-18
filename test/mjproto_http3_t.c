#include "mjproto_http.h"
#include "mjconnb.h"
#include "mjsock.h"
#include "mjlf.h"
#include <stdio.h>

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
  if (count > 10000) {
    mjlf server = mjconnb_get_obj(conn, "server");
    mjlf_set_stop(server, true);
  }
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
  int sfd = mjsock_tcp_server(7879);
  if (sfd < 0) {
    printf("mjsock_tcp_server error");
    return 1;
  }
  mjlf srv = mjlf_new(sfd, 4);
  mjlf_set_init(srv, http_mjlf_init, urls);
  mjlf_set_routine(srv, http_mjlf_routine);
  mjlf_run(srv);
  mjlf_delete(srv);
  return 0;
}

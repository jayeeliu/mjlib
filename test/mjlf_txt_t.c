#include "mjsock.h"
#include "mjlf_txt.h"
#include <stdio.h>

void* test_routine(mjlf_txt_cmd cmd) {
  mjconnb_writes(cmd->conn, "+ OK\r\n");
  return NULL;
}

void* abc_routine(mjlf_txt_cmd cmd) {
  mjconnb_writes(cmd->conn, "+ abc is here\r\n");
  return NULL;
}

struct mjlf_txt_cmdlist cmdlist[] = {
  {"test", test_routine},
  {"abc", abc_routine},
  {"quit", NULL},
  {NULL, NULL},
};

int main() {
  int sfd = mjsock_tcp_server(7879);
  if (sfd < 0) {
    printf("mjsock_tcp server error");
    return 0;
  }

  mjlf srv = mjlf_txt_new(sfd, 10, cmdlist);
  if (!srv) {
    printf("mjlf_txt_new error");
    return 0;
  }
  mjlf_run(srv);
  mjlf_delete(srv);
  return 0;
}

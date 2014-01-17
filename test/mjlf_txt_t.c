#include "mjsock.h"
#include "mjlf_txt.h"
#include <stdio.h>

void* hello_routine(mjlf_txt_cmd cmd) {
  mjconnb_writes(cmd->conn, "+ OK Server Ready\r\n");
  return NULL;
}

void* test_routine(mjlf_txt_cmd cmd) {
  mjconnb_writes(cmd->conn, "+ OK\r\n");
  return NULL;
}

void* abc_routine(mjlf_txt_cmd cmd) {
  mjconnb_writes(cmd->conn, "+ abc is here\r\n");
  return NULL;
}

void* shut_routine(mjlf_txt_cmd cmd) {
  mjlf_set_stop(cmd->srv, true);
  cmd->finished = true;
  return NULL;
}

void* timeout_routine(mjlf_txt_cmd cmd) {
  mjconnb_writes(cmd->conn, "+ Timeout Bye!\r\n");
  cmd->finished = true;
  return NULL;
}

struct mjlf_txt_cmdlist cmdlist[] = {
  {"test", test_routine},
  {"abc", abc_routine},
  {"shut", shut_routine},
  {"quit", NULL},
  {NULL, NULL},
};

struct mjlf_txt_ctl ctl = {
  hello_routine,
  timeout_routine,
  NULL,
  NULL,
  cmdlist,
};

int main() {
  int sfd = mjsock_tcp_server(7879);
  if (sfd < 0) {
    printf("mjsock_tcp server error");
    return 0;
  }

  mjlf srv = mjlf_txt_new(sfd, 10, &ctl);
  if (!srv) {
    printf("mjlf_txt_new error");
    return 0;
  }
  mjlf_set_timeout(srv, 3000, 3000);
  mjlf_run(srv);
  mjlf_delete(srv);
  return 0;
}

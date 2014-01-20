#include "mjsock.h"
#include "mjlf_pb.h"
#include <stdio.h>

static void* pb_routine(mjlf_pb_cmd cmd) {
  cmd->finished = true;
  return NULL;
}

struct mjlf_pb_ctl ctl = {
  NULL,
  NULL,
  NULL,
  pb_routine,
};

int main() {
  int sfd = mjsock_tcp_server(7879);
  if (sfd < 0) {
    printf("server create error\n");
    return 0;
  }
  mjlf srv = mjlf_pb_new(sfd, 10, &ctl);
  if (!srv) {
    printf("mjlf_pb_new error\r\n");
    return 0;
  }
  mjlf_run(srv);
  mjlf_delete(srv);
  return 0;
}

#include "mjseg.h"
#include "mjsock.h"
#include "mjcomm.h"
#include "mjlf_txt.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void* seg_routine(void* arg) {
  mjlf_txt_cmd cmd = arg;
  mjthread thread = mjconnb_get_obj(cmd->conn, "thread");
  mjseg seg = mjthread_get_obj(thread, "seg");

  if (cmd->args->len < 2) {
    mjconnb_writes(cmd->conn, "+ seg command error\r\n");
    goto out1;
  }

  // get string
  mjstr str = mjstr_new(80);
  if (!str) {
    mjconnb_writes(cmd->conn, "+ seg inner error\r\n");
    goto out1;
  }
  for (int i = 1; i < cmd->args->len; i++) {
    mjstr tmp = mjslist_get(cmd->args, i);
    mjstr_cats(str, " ");
    mjstr_cat(str, tmp);
  }
  mjslist slist = mjseg_segment(seg, str->data);
  if (!slist) {
    mjconnb_writes(cmd->conn, "+ seg innner error\r\n");
    goto out2;
  }
  // show result
  mjconnb_writes(cmd->conn, "{");
  for (int i = 0; i < slist->len; i++) {
    if (i) mjconnb_writes(cmd->conn, ", ");
    mjconnb_write(cmd->conn, mjslist_get(slist, i));
  }
  mjconnb_writes(cmd->conn, "}\r\n");
  mjconnb_writes(cmd->conn, "+200 OK\r\n");
  mjslist_delete(slist);

out2:
  mjstr_delete(str);
out1:
  cmd->finished = true;
  return NULL;
}

void* thread_init(void* thread) {
  mjthread_set_obj(thread, "seg", mjthread_get_iarg(thread), NULL);
  return NULL;
}

struct mjlf_txt_cmdlist cmdlist[] = {
  {"seg", seg_routine},
  {NULL, NULL},
};

static inline void usage(char *cmd) {
  fprintf(stderr, "Usage: %s [-p port] <-c config>\n", cmd);
}

int main(int argc, char* argv[]) {
  int port = 7879;
  char* conf = NULL;
  int ch;
  opterr = 0;
  while ((ch = getopt(argc, argv, "c:p:")) != -1) {
    switch (ch) {
      case 'c':
        conf = optarg;
        break;
      case 'p':
        port = atoi(optarg);
        break;
      default:
        usage(argv[0]);
        return 1;
    }
  }
  if (!conf) {
    usage(argv[0]);
    return 1;
  }
  // init mjseg
  mjseg seg = mjseg_new(conf);
  if (!seg) {
    fprintf(stderr, "[ERROR] mjseg create error\n");
    return 1;
  }
  // tcp server socket
  int sfd = mjsock_tcp_server(port);
  if (sfd < 0) {
    fprintf(stderr, "[ERROR] socket create error\n");
    mjseg_delete(seg);
    return 1;
  }
  // create mjlf server
  mjlf srv = mjlf_txt_new(sfd, get_cpu_count(), cmdlist);
  if (!srv) {
    fprintf(stderr, "[ERROR] mjlf create error\n");
    mjseg_delete(seg);
    return 1;
  }
  mjlf_set_thread_init(srv, thread_init, seg);
  mjlf_run(srv);
  mjlf_delete(srv);
  mjseg_delete(seg);
  return 0;
}

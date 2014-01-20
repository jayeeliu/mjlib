#include "mjlf_pb.h"
#include "mjlog.h"
#include <stdlib.h>

static void* mjlf_pb_init(mjlf srv, void* arg) {
  mjlf_set_local(srv, "ctl", arg, NULL);
  return NULL;
}

static bool mjlf_pb_readbytes(mjlf_pb_cmd cmd, mjlf_pb_ctl ctl, int llen) {
  mjstr_clean(cmd->line);
  int ret = mjconnb_readbytes(cmd->conn, cmd->line, llen);
  if (ret == -2) {
    if (ctl->_readTimeout) {
      ctl->_readTimeout(cmd);
    } else {
      cmd->finished = true;
    }
    return false;
  } else if (ret < 0) {
    if (ctl->_innerErr) {
      ctl->_innerErr(cmd);
    } else {
      cmd->finished = true;
    }
    return false;
  } else if (ret == 0) {
    cmd->finished = true;
    return false;
  }
  return true;
}

static void mjlf_pb_runctl(mjlf_pb_cmd cmd, mjlf_pb_ctl ctl) {
  // read cmd len
  bool ret = mjlf_pb_readbytes(cmd, ctl, sizeof(uint32_t));
  if (!ret) return;
  // read cmd
  int llen = 0;
  for(int i=0; i < cmd->line->len; i++) {
    llen = (llen << 8) + cmd->line->data[i];
  }
  ret = mjlf_pb_readbytes(cmd, ctl, llen);
  if (!ret) return;
  if (ctl->_cmdproc) {
    ctl->_cmdproc(cmd);
  } else {
    MJLOG_ERR("Oops no cmdproc");
    cmd->finished = true;
  }
  return;
}

static void* mjlf_pb_routine(mjlf srv, mjthread thread, mjconnb conn) {
  mjlf_pb_ctl ctl = mjlf_get_local(srv, "ctl");
  if (!ctl) {
    MJLOG_ERR("Ctl Not Found");
    return NULL;
  }
  struct mjlf_pb_cmd cmd = {0};
  cmd.srv = srv;
  cmd.thread = thread;
  cmd.conn = conn;
  if (ctl->_connAccept) ctl->_connAccept(&cmd);
  if (cmd.finished) return NULL;
  // alloc line
  cmd.line = mjstr_new(80);
  if (!cmd.line) {
    if (ctl->_innerErr) {
      ctl->_innerErr(&cmd);
    } else {
      cmd.finished = true;
    }
  }
  // enter cmd loop
  while (!cmd.finished) {
    mjlf_pb_runctl(&cmd, ctl);
  }
  return NULL;
}

mjlf mjlf_pb_new(int sfd, int threadNum, mjlf_pb_ctl ctl) {
  if (!ctl || !ctl->_cmdproc) return NULL;
  mjlf srv = mjlf_new(sfd, threadNum);
  if (!srv) {
    MJLOG_ERR("mjlf_new error");
    return NULL;
  }
  mjlf_set_init(srv, mjlf_pb_init, ctl);
  mjlf_set_task(srv, mjlf_pb_routine);
  return srv;
}

#include <string.h>
#include "mjlf_txt.h"
#include "mjlog.h"

/*
===============================================================================
mjlf_txt_init(server routine)
  init proto txt.
===============================================================================
*/
static void* mjlf_txt_init(mjlf srv, void* arg) {
  mjlf_set_local(srv, "ctl", arg, NULL);
  return NULL;
}

/*
===============================================================================
mjplf_txt_runcmd
  run txt protocol command(run txt routine)
===============================================================================
*/
static void mjlf_txt_runctl(mjlf_txt_cmd cmd, mjlf_txt_ctl ctl) {
  mjstr_clean(cmd->line);
  mjslist_clean(cmd->args);

  // read command
  int ret = mjconnb_readuntil(cmd->conn, "\r\n", cmd->line);
  if (ret == -2) {
    if (ctl->_readTimeout) {
      ctl->_readTimeout(cmd);
    } else { // default action for timeout
      cmd->finished = true;
    }
    return;
  } else if (ret < 0) {
    if (ctl->_innerErr) {
      ctl->_innerErr(cmd);
    } else { // default action for inner error
      cmd->finished = true;
    }
    return;
  } else if (ret == 0) {
    cmd->finished = true;
    return;
  }

  mjstr_split(cmd->line, " ", cmd->args);
  if (cmd->args->len < 1) {
    if (ctl->_cmdErr) ctl->_cmdErr(cmd);
    return;
  }

  cmd->cmdname = mjslist_get(cmd->args, 0);
  mjstr_strim(cmd->cmdname);
  // run routine
  for (int i = 0; ctl->_cmdlist[i].cmdname != NULL; i++) {
    if (strcasecmp(ctl->_cmdlist[i].cmdname, cmd->cmdname->data)) continue;
    if (ctl->_cmdlist[i].proc) {
      (*(ctl->_cmdlist[i].proc))(cmd);
    } else {
      cmd->finished = true;
    }
    return;
  }  
  if (ctl->_cmdErr) ctl->_cmdErr(cmd);
  return;
}

/*
===============================================================================
mjlf_txt_routine
===============================================================================
*/
static void* mjlf_txt_routine(mjlf srv, mjthread thread, mjconnb conn) {
	mjlf_txt_ctl ctl = mjlf_get_local(srv, "ctl");
  if (!ctl) {
		MJLOG_ERR("Command Ctl Not Found");
    return NULL;
  } 

  // creaet command struct
  struct mjlf_txt_cmd cmd = {0};
  cmd.srv = srv;
  cmd.thread = thread;
  cmd.conn = conn;
  if (ctl->_connAccept) ctl->_connAccept(&cmd);
  if (cmd.finished) return NULL;

  // alloc buffer
  cmd.args = mjslist_new();
  cmd.line = mjstr_new(80);
  if (!cmd.args || !cmd.line) {
    if (ctl->_innerErr) {
      ctl->_innerErr(&cmd);
    } else { // default action for inner error
      cmd.finished = true;
    }
  }

	while (!cmd.finished) {
    mjlf_txt_runctl(&cmd, ctl);
  }

  mjslist_delete(cmd.args);
  mjstr_delete(cmd.line);
	return NULL;
}

/*
===============================================================================
mjlf_txt_new
  create mjlf_txt server
===============================================================================
*/
mjlf mjlf_txt_new(int sfd, int threadNum, mjlf_txt_ctl ctl) {
  if (!ctl || !ctl->_cmdlist) return NULL;
  mjlf srv = mjlf_new(sfd, threadNum);
  if (!srv) {
    MJLOG_ERR("mjlf_new error");
    return NULL;
  }
  mjlf_set_init(srv, mjlf_txt_init, ctl);
  mjlf_set_task(srv, mjlf_txt_routine);
  return srv;
}

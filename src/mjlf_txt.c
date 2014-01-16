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
  mjlf_set_local(srv, "cmdlist", arg, NULL);
  return NULL;
}

/*
===============================================================================
mjplf_txt_runcmd
  run txt protocol command(run txt routine)
===============================================================================
*/
static void mjlf_txt_runcmd(mjconnb conn, mjlf_txt_cmd cmd, 
    mjlf_txt_cmdlist cmdlist) {
  // read data
  mjstr data = mjstr_new(80);
  if (!data) {
		mjconnb_writes(conn, "+ Server Inner Error\r\n");
    MJLOG_ERR("Data Create Error");
    goto failout0;
  }

  // read command
  int ret = mjconnb_readuntil(conn, "\r\n", data);
  if (ret == -2) {
    mjconnb_writes(conn, "+ Read Timeout\r\n");
    goto failout1;
  } else if (ret < 0) {
    mjconnb_writes(conn, "+ Read Error\r\n");
    goto failout1;
  } else if (ret == 0) {
    MJLOG_ERR("Peer Closed");
    goto failout1;
  }

  // split string
  mjslist slist = mjslist_new();
  if (!slist) {
		mjconnb_writes(conn, "+ Inner Error\r\n");
    MJLOG_ERR("mjslist create error");
    goto failout1;
  }
  mjstr_split(data, " ", slist);
  if (slist->len < 1) {
    mjconnb_writes(conn, "+ Command Error\r\n");
    goto failout2;
  }
  // get command
  cmd->cmdname = mjslist_get(slist, 0);
  mjstr_strim(cmd->cmdname);
  cmd->args = slist;
  cmd->conn = conn;

  // run routine
  for (int i = 0; cmdlist[i].cmdname != NULL; i++) {
    if (strcasecmp(cmdlist[i].cmdname, cmd->cmdname->data)) continue;
    if (cmdlist[i].Routine) {
      (*cmdlist[i].Routine)(cmd);
    } else {
      cmd->finished = true;
    }
    mjslist_delete(slist);
    mjstr_delete(data);
    return;
  }  
  mjconnb_writes(conn, "+ Wrong command\r\n");
failout2:
  mjslist_delete(slist);
failout1:
  mjstr_delete(data);	
failout0:
  cmd->finished = true;
  return;
}

/*
===============================================================================
mjlf_txt_routine(conn_routine)
  run by mjlf. the main routine of mjproto_txt.
  arg is mjconnb
===============================================================================
*/
static void* mjlf_txt_routine(void* conn) {
	mjlf srv = mjconnb_get_obj(conn, "server");
	if (!srv) {
		mjconnb_writes(conn, "+ Server Inner Error\r\n");
		MJLOG_ERR("No Server Found");
		return NULL;
	}

  // get routine_list
	mjlf_txt_cmdlist cmdlist = mjlf_get_local(srv, "cmdlist");
  if (!cmdlist) {
    mjconnb_writes(conn, "+ No Command List\r\n");
		MJLOG_ERR("No Command List Found");
    return NULL;
  } 

	// alloc mjlf_txt_cmd, run routine till finished
  struct mjlf_txt_cmd cmd = {0};
	while (!cmd.finished) {
    mjlf_txt_runcmd(conn, &cmd, cmdlist);
  }
	return NULL;
}

/*
===============================================================================
mjlf_txt_new
  create mjlf_txt server
===============================================================================
*/
mjlf mjlf_txt_new(int sfd, int max_thread, mjlf_txt_cmdlist cmdlist) {
  mjlf srv = mjlf_new(sfd, max_thread);
  if (!srv) {
    MJLOG_ERR("mjlf_new error");
    return NULL;
  }
  mjlf_set_init(srv, mjlf_txt_init, cmdlist);
  mjlf_set_routine(srv, mjlf_txt_routine);
  return srv;
}

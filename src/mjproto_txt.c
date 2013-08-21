#include <string.h>
#include "mjproto_txt.h"
#include "mjlog.h"
#include "mjlf.h"

void* mjproto_txt_init(void* arg) {
	mjlf server = (mjlf) arg;
	mjproto_txt_routine_list routine_list = server->srv_init_arg;
	if (routine_list) {
		mjlf_set_obj(server, "routine_list", routine_list, NULL);
	}
	return NULL;
}

/*
===============================================================================
mjTxt_RunCmd
  run txt protocol command
===============================================================================
*/
static bool mjproto_txt_run_cmd(mjconnb conn, mjproto_txt_data cmd_data,
		mjproto_txt_routine_list routine_list) {
  // read data
  mjstr data = mjstr_new();
  if (!data) {
		mjconnb_writes(conn, "+ inner error\r\n");
    MJLOG_ERR("data create error");
    return false;  
  }
  int ret = mjconnb_readuntil(conn, "\r\n", data);
  if (ret == -2) {
    mjconnb_writes(conn, "+ read timetout\r\n");
    goto failout1;
  } else if (ret < 0) {
    mjconnb_writes(conn, "+ read error\r\n");
    goto failout1;
  } else if (ret == 0) {
    MJLOG_ERR("peer closed");
    goto failout1;
  }
  // split string
  mjstrlist str_list = mjstrlist_new();
  if (!str_list) {
		mjconnb_writes(conn, "+ inner error\r\n");
    MJLOG_ERR("mjstrlist create error");
    goto failout1;
  }
  mjstr_split(data, " ", str_list);
  if (str_list->length < 1) {
    mjconnb_writes(conn, "+ command error\r\n");
    goto failout2;
  }
  // get tag and cmd
  cmd_data->cmd = mjstrlist_get(str_list, 0);
  mjstr_strim(cmd_data->cmd);
  cmd_data->args = str_list;
  cmd_data->conn = conn;
  // run routine
  for (int i = 0; routine_list[i].cmd != NULL; i++) {
    // not equal continue
    if (strcasecmp(routine_list[i].cmd, cmd_data->cmd->data)) continue;
    // run routine
    if (routine_list[i].Routine) (*routine_list[i].Routine)(cmd_data);
    // clean and return
    mjstrlist_delete(str_list);
    mjstr_delete(data);
    return true;
  }  
  mjconnb_writes(conn, "+ wrong command\r\n");
  // release resource
failout2:
  mjstrlist_delete(str_list);
failout1:
  mjstr_delete(data);	
  return false;
}

void* mjproto_txt_routine(void* arg) {
  // sanity check
	mjconnb conn = (mjconnb) arg;
  if (!conn) {
    MJLOG_ERR("conn is null");
    return NULL;
  }
	// get server and routine list
	mjlf server = mjconnb_get_obj(conn, "server");
	if (!server) {
		mjconnb_writes(conn, "+ inner error\r\n");
		MJLOG_ERR("no server found");
		return NULL;
	}
	mjproto_txt_routine_list routine_list = mjlf_get_obj(server, "routine_list");
  if (!routine_list) {
    mjconnb_writes(conn, "+ no command list\r\n");
		MJLOG_ERR("no command list set");
    return NULL;
  } 
	// alloc mjproto_txt_data
  struct mjproto_txt_data cmd_data;
	cmd_data.cmd 	= NULL;
	cmd_data.args = NULL;
	cmd_data.conn	= NULL;
	cmd_data.finished = false;
	// run routine
	while (mjproto_txt_run_cmd(conn, &cmd_data, routine_list) && !cmd_data.finished);
	return NULL;
}

bool mjproto_txt_finished(mjproto_txt_data cmd_data) {
	if (!cmd_data) {
		MJLOG_ERR("cmd_data is null");
		return false;
	}
	cmd_data->finished = true;
	return true;
}

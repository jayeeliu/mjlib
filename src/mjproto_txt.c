#include <string.h>
#include "mjproto_txt.h"
#include "mjlog.h"
#include "mjlf.h"

/*
===============================================================================
mjproto_txt_init(server routine)
  init proto txt.
===============================================================================
*/
void* mjproto_txt_init(void* arg) {
  mjlf srv = (mjlf) arg;
  mjproto_txt_rtlist rlist = mjlf_get_iarg(srv);
  if (rlist) mjlf_set_obj(srv, "routine_list", rlist, NULL);
  return NULL;
}

/*
===============================================================================
mjproto_txt_run_cmd
  run txt protocol command(run txt routine)
===============================================================================
*/
static bool mjproto_txt_run_cmd(mjconnb conn, mjproto_txt_data cdata,
		mjproto_txt_rtlist rtlist) {
  // read data
  mjstr data = mjstr_new(80);
  if (!data) {
		mjconnb_writes(conn, "+ inner error\r\n");
    MJLOG_ERR("data create error");
    return false;  
  }
  // read command
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
  mjslist slist = mjslist_new();
  if (!slist) {
		mjconnb_writes(conn, "+ inner error\r\n");
    MJLOG_ERR("mjslist create error");
    goto failout1;
  }
  mjstr_split(data, " ", slist);
  if (slist->len < 1) {
    mjconnb_writes(conn, "+ command error\r\n");
    goto failout2;
  }
  // get command
  cdata->cmd = mjslist_get(slist, 0);
  mjstr_strim(cdata->cmd);
  cdata->args = slist;
  cdata->conn = conn;
  // run routine
  for (int i = 0; rtlist[i].cmd != NULL; i++) {
    // not equal continue
    if (strcasecmp(rtlist[i].cmd, cdata->cmd->data)) continue;
    // run routine
    if (rtlist[i].Routine) (*rtlist[i].Routine)(cdata);
    // clean and return
    mjslist_delete(slist);
    mjstr_delete(data);
    return true;
  }  
  mjconnb_writes(conn, "+ wrong command\r\n");
failout2:
  mjslist_delete(slist);
failout1:
  mjstr_delete(data);	
  return false;
}

/*
===============================================================================
mjproto_txt_routine(conn_routine)
  run by mjlf. the main routine of mjproto_txt.
  arg is mjconnb
===============================================================================
*/
void* mjproto_txt_routine(void* arg) {
  if (!arg) return NULL;

	mjconnb conn = (mjconnb) arg;
	mjlf srv = mjconnb_get_obj(conn, "server");
	if (!srv) {
		mjconnb_writes(conn, "+ inner error\r\n");
		MJLOG_ERR("no server found");
		return NULL;
	}

  // get routine_list
	mjproto_txt_rtlist rtlist = mjlf_get_obj(srv, "routine_list");
  if (!rtlist) {
    mjconnb_writes(conn, "+ no command list\r\n");
		MJLOG_ERR("no command list found");
    return NULL;
  } 

	// alloc mjproto_txt_data, run routine till finished
  struct mjproto_txt_data cdata = {0};
	while (mjproto_txt_run_cmd(conn, &cdata, rtlist) && !cdata.finished);
	return NULL;
}

/*
===============================================================================
mjproto_txt_finished
  set conn finished
===============================================================================
*/
bool mjproto_txt_finished(mjproto_txt_data cdata) {
	if (!cdata) return false;
	cdata->finished = true;
	return true;
}

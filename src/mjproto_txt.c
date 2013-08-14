#include <string.h>
#include "mjproto_txt.h"
#include "mjlog.h"

/*
===============================================================================
mjTxt_RunCmd
  run txt protocol command
===============================================================================
*/
bool mjTxt_RunCmd(PROTO_TXT_ROUTINE routineList[], int length, mjconnb conn) {
  // sanity check
  if (!conn) {
    MJLOG_ERR("conn is null");
    return false;
  }
  if (!routineList) {
    mjconnb_writes(conn, "+ no command list\r\n");
    return false;
  } 
  // read data
  mjstr data = mjstr_new();
  if (!data) {
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
  mjstrlist strList = mjstrlist_new();
  if (!strList) {
    MJLOG_ERR("mjstrlist create error");
    goto failout1;
  }
  mjstr_split(data, " ", strList);
  if (strList->length < 2) {
    mjconnb_writes(conn, "+ command error\r\n");
    goto failout2;
  }
  // get tag and cmd
  struct mjProtoTxtData cmdData;
  cmdData.tag = mjstrlist_get(strList, 0);
  mjstr_strim(cmdData.tag);
  cmdData.cmd = mjstrlist_get(strList, 1);
  mjstr_strim(cmdData.cmd);
  cmdData.arg = strList;
  cmdData.conn = conn;
  // run routine
  for (int i = 0; i < length; i++) {
    // not equal continue
    if (strcasecmp(routineList[i].cmd, (cmdData.cmd)->data)) continue;
    // run routine
    if (routineList[i].Routine) (*routineList[i].Routine)(&cmdData);
    // clean and return
    mjstrlist_delete(strList);
    mjstr_delete(data);
    return true;
  }  
  mjconnb_writes(conn, "+ wrong command\r\n");
  // release resource
failout2:
  mjstrlist_delete(strList);
failout1:
  mjstr_delete(data);
  return false;
}

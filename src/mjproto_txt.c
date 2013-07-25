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
    mjconnb_WriteS(conn, "+ no command list\r\n");
    return false;
  } 
  // read data
  mjStr data = mjStr_New();
  if (!data) {
    MJLOG_ERR("data create error");
    return false;  
  }
  int ret = mjconnb_ReadUntil(conn, "\r\n", data);
  if (ret == -2) {
    mjconnb_WriteS(conn, "+ read timetout\r\n");
    goto failout1;
  } else if (ret < 0) {
    mjconnb_WriteS(conn, "+ read error\r\n");
    goto failout1;
  } else if (ret == 0) {
    MJLOG_ERR("peer closed");
    goto failout1;
  }
  // split string
  mjStrList strList = mjStrList_New();
  if (!strList) {
    MJLOG_ERR("mjstrlist create error");
    goto failout1;
  }
  mjStr_Split(data, " ", strList);
  if (strList->length < 2) {
    mjconnb_WriteS(conn, "+ command error\r\n");
    goto failout2;
  }
  // get tag and cmd
  struct mjProtoTxtData cmdData;
  cmdData.tag = mjStrList_Get(strList, 0);
  mjStr_Strim(cmdData.tag);
  cmdData.cmd = mjStrList_Get(strList, 1);
  mjStr_Strim(cmdData.cmd);
  cmdData.arg = strList;
  cmdData.conn = conn;
  // run routine
  for (int i = 0; i < length; i++) {
    // not equal continue
    if (strcasecmp(routineList[i].cmd, (cmdData.cmd)->data)) continue;
    // run routine
    if (routineList[i].Routine) (*routineList[i].Routine)(&cmdData);
    // clean and return
    mjStrList_Delete(strList);
    mjStr_Delete(data);
    return true;
  }  
  mjconnb_WriteS(conn, "+ wrong command\r\n");
  // release resource
failout2:
  mjStrList_Delete(strList);
failout1:
  mjStr_Delete(data);
  return false;
}

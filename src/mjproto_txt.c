#include <string.h>
#include "mjproto_txt.h"
#include "mjlog.h"

bool mjTxt_RunCmd(PROTO_TXT_ROUTINE routineList[], int length, mjConnB conn)
{
  // sanity check
  if (!conn) {
    MJLOG_ERR("conn is null");
    return false;
  }
  if (!routineList) {
    mjConnB_WriteS(conn, "+ no command list\r\n");
    return false;
  } 
  // read data
  mjStr data = mjStr_New();
  if (!data) {
    MJLOG_ERR( "data create error" );
    return false;  
  }
  mjConnB_ReadUntil(conn, "\r\n", data);
  // split string
  mjStrList strList = mjStrList_New();
  if (!strList) {
    MJLOG_ERR("mjstrlist create error");
    return false;
  }
  mjStr_Split(data, " ", strList);
  if (strList->length < 2) {
    mjConnB_WriteS(conn, "+ command error\r\n");
    return false;
  }
  // get tag and cmd
  struct mjProtoTxtData cmdData;
  cmdData.tag = mjStrList_Get(strList, 0);
  cmdData.cmd = mjStrList_Get(strList, 1);
  mjStr_Strim(cmdData.cmd);
  cmdData.conn = conn;
  // run routine
  for (int i = 0; i < length; i++) {
    if (!strcmp(routineList[i].cmd, (cmdData.cmd)->data)) {
      if (routineList[i].Routine) {
        (*routineList[i].Routine)(&cmdData);
      }
      mjStrList_Delete(strList);
      mjStr_Delete(data);
      return true;
    }
  }
  mjConnB_WriteS(conn, "+ wrong command\r\n");
  // release resource
  mjStrList_Delete(strList);
  mjStr_Delete(data);
  return false;
}

#ifndef _MJPROTO_TXT_H
#define _MJPROTO_TXT_H

#include "mjproc.h"
#include "mjstr.h"
#include "mjconnb.h"

struct mjProtoTxtData {
  mjstr     tag;
  mjstr     cmd;
  mjstrlist arg;
  mjconnb   conn;
};

typedef struct PROTO_TXT_ROUTINE {
  const char  *cmd;
  mjProc      Routine;
} PROTO_TXT_ROUTINE;

extern bool   mjTxt_RunCmd(PROTO_TXT_ROUTINE routineList[], int length, 
                    mjconnb conn);

#endif

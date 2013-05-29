#ifndef _MJPROTO_TXT_H
#define _MJPROTO_TXT_H

#include "mjproc.h"
#include "mjstr.h"
#include "mjconnb.h"

struct mjProtoTxtData {
  mjStr     tag;
  mjStr     cmd;
  mjStrList arg;
  mjConnB   conn;
};

typedef struct PROTO_TXT_ROUTINE {
  const char* cmd;
  mjProc      Routine;
} PROTO_TXT_ROUTINE;

extern bool mjTxt_RunCmd(PROTO_TXT_ROUTINE routineList[], int length, 
                    mjConnB conn);

#endif

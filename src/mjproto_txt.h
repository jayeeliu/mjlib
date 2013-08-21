#ifndef _MJPROTO_TXT_H
#define _MJPROTO_TXT_H

#include "mjproc.h"
#include "mjstr.h"
#include "mjconnb.h"

struct mjproto_txt_data {
  mjstr     cmd;
  mjstrlist args;
  mjconnb   conn;
};

typedef struct PROTO_TXT_ROUTINE {
  const char  *cmd;
  mjProc      Routine;
} PROTO_TXT_ROUTINE;

extern bool   mjtxt_run_cmd(PROTO_TXT_ROUTINE routineList[], int length, mjconnb conn);

#endif

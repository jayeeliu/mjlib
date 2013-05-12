#ifndef MJPROTO_TXT
#define MJPROTO_TXT

#include "mjproc.h"
#include "mjstr.h"
#include "mjconnb.h"

struct mjProtoTxtData {
  mjStr   tag;
  mjStr   cmd;
  mjStr   arg;
  mjConnB conn;
};

typedef struct PROTO_TXT_ROUTINE {
    const char* cmd;
    mjProc      Routine;
} PROTO_TXT_ROUTINE;

extern bool mjTxt_RunCmd( PROTO_TXT_ROUTINE routineList[], int length, mjConnB conn );

#endif

#ifndef _MJPROTO_TXT_H
#define _MJPROTO_TXT_H

#include "mjproc.h"
#include "mjstr.h"
#include "mjconnb.h"
#include "mjlf.h"

struct mjlf_txt_cmd {
  mjstr   cmdname;
  mjslist args;
  mjconnb conn;
  bool    finished;
};
typedef struct mjlf_txt_cmd* mjlf_txt_cmd;

struct mjlf_txt_cmdlist {
  const char* cmdname;
  mjProc      Routine;
};
typedef struct mjlf_txt_cmdlist* mjlf_txt_cmdlist;

extern mjlf mjlf_txt_new(int sfd, int max_thread, mjlf_txt_cmdlist cmdlist);

#endif

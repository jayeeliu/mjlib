#ifndef _MJLF_TXT_H
#define _MJLF_TXT_H

#include "mjproc.h"
#include "mjstr.h"
#include "mjconnb.h"
#include "mjlf.h"

#define MJLF_TXT_NOLIMIT -1

struct mjlf_txt_cmd {
  mjstr     cmdname;
  mjslist   args;       // args split line
  mjlf      srv;
  mjthread  thread;
  mjconnb   conn;
  mjstr     line;       // read line buf
  bool      finished;
  void*     private;
};
typedef struct mjlf_txt_cmd* mjlf_txt_cmd;

typedef void* (*mjlftxtProc)(mjlf_txt_cmd);

struct mjlf_txt_cmdlist {
  const char* cmdname;        // command name
  int minArg;                 // min command args
  int maxArg;                 // max command args
  mjlftxtProc proc;           // command proc
  mjlftxtProc errproc;        // run when args error
};
typedef struct mjlf_txt_cmdlist* mjlf_txt_cmdlist; // 0 for hello message

struct mjlf_txt_ctl {
  mjlftxtProc       _connAccept;
  mjlftxtProc       _readTimeout;
  mjlftxtProc       _innerErr;
  mjlftxtProc       _cmdErr;
  mjlf_txt_cmdlist  _cmdlist;
};
typedef struct mjlf_txt_ctl* mjlf_txt_ctl;

extern mjlf mjlf_txt_new(int sfd, int threadNum, mjlf_txt_ctl ctl);

#endif

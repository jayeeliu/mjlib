#ifndef __MJLF_PB_H
#define __MJLF_PB_H

#include "mjlf.h"

struct mjlf_pb_cmd {
  mjlf      srv;
  mjthread  thread;
  mjconb   conn;
  mjstr     line;       // read line buffer;
  bool      finished;
  void*     private;
};
typedef struct mjlf_pb_cmd* mjlf_pb_cmd;

typedef void* (*mjlfpbProc)(mjlf_pb_cmd);

struct mjlf_pb_ctl {
  mjlfpbProc  _connAccept;
  mjlfpbProc  _readTimeout;
  mjlfpbProc  _innerErr;
  mjlfpbProc  _cmdproc;
};
typedef struct mjlf_pb_ctl* mjlf_pb_ctl;

extern mjlf mjlf_pb_new(int sfd, int threadNum, mjlf_pb_ctl ctl);

#endif

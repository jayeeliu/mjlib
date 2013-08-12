#ifndef _MJTCPSRV_H
#define _MJTCPSRV_H

#include "mjev.h"

#define MJTCPSRV_STANDALONE 0
#define MJTCPSRV_INNER      1

struct mjtcpsrv {
  int     sfd;          // socket, accept for standalone, read for inner
  int     type;         // tcpsrv type, standalone or inner
  bool    stop;         // server stop
  mjev    ev;           // event loop
  mjProc  Routine;      // server routine
  void*   mainSrv;      // used in inner mode, point to mainSrv

  mjProc  InitSrv;      // init Server proc
  mjProc  ExitSrv;      // exit Server proc
  void*   srv_local;
};
typedef struct mjtcpsrv* mjtcpsrv;

extern void*    mjtcpsrv_run(void *arg);
extern bool     mjtcpsrv_set_stop(mjtcpsrv srv, int value);

extern mjtcpsrv mjtcpsrv_new(int sfd, mjProc Routine, mjProc InitSrv,
    mjProc ExitSrv, int type);
extern void*    mjtcpsrv_delete(void *arg);

#endif

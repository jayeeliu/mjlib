#ifndef _MJMAINSRV_H
#define _MJMAINSRV_H

#include <stdbool.h>
#include "mjev.h"
#include "mjthreadpool.h"
#include "mjtcpsrv.h"

#define MAX_SERVER_NUM  64

struct mjMainSrv {
  int           sfd;
  int           stop;
  
  int           workerThreadNum;
  mjThreadPool  workerThreadPool;

  mjProc        srvRoutine;
  int           srvNum;
  int           srvNotify[MAX_SERVER_NUM];
  mjTcpSrv      srv[MAX_SERVER_NUM];
  mjThread      srvThread[MAX_SERVER_NUM];

  mjProc        InitSrv;
  mjProc        ExitSrv;

  void          *private;
  mjProc        FreePrivate;
};
typedef struct mjMainSrv* mjMainSrv;

extern bool       mjMainSrv_Async(mjMainSrv srv, mjProc Routine, void *rdata, 
                    mjEV ev, mjProc CallBack, void *cdata);
extern bool       mjMainSrv_Run(mjMainSrv srv);
extern bool       mjMainSrv_SetPrivate(mjMainSrv srv, void *private, 
                    mjProc FreePrivate);
extern bool       mjMainSrv_SetSrvProc(mjMainSrv srv, mjProc InitSrv, 
                    mjProc ExitSrv);
extern bool       mjMainSrv_SetStop(mjMainSrv srv, int value);

extern mjMainSrv  mjMainSrv_New(int sfd, mjProc srvRoutine, 
                    int workerThreadNum);
extern bool       mjMainSrv_Delete(mjMainSrv srv);

#endif

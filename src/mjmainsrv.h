#ifndef _MJMAINSRV_H
#define _MJMAINSRV_H

#include <stdbool.h>
#include "mjev.h"
#include "mjthreadpool.h"
#include "mjtcpsrv.h"

#define MAX_SERVER_NUM  64

struct mjmainsrv {
  int           sfd;
  int           stop;
  
  int           workerThreadNum;
  mjthreadpool  workerThreadPool;

  int           srvNum;
  int           srvNotify[MAX_SERVER_NUM];
  mjtcpsrv      srv[MAX_SERVER_NUM];
  mjthread      srvThread[MAX_SERVER_NUM];

  mjProc        InitSrv;
  mjProc        ExitSrv;
};
typedef struct mjmainsrv* mjmainsrv;

extern bool       mjmainsrv_async(mjmainsrv srv, mjProc Routine, void *rdata, 
                    mjev ev, mjProc CallBack, void *cdata);
extern bool       mjmainsrv_run(mjmainsrv srv);
extern bool       mjmainsrv_set_srvproc(mjmainsrv srv, mjProc InitSrv, mjProc ExitSrv);
extern bool       mjmainsrv_set_stop(mjmainsrv srv, int value);

extern mjmainsrv  mjmainsrv_new(int sfd, mjProc srvRoutine, int workerThreadNum);
extern bool       mjmainsrv_delete(mjmainsrv srv);

#endif

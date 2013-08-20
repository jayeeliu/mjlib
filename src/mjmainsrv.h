#ifndef _MJMAINSRV_H
#define _MJMAINSRV_H

#include <stdbool.h>
#include "mjev.h"
#include "mjthreadpool.h"
#include "mjtcpsrv.h"

#define MAX_SERVER_NUM  64

struct mjmainsrv {
  int           _sfd;
  int           _stop;
  
  mjthreadpool  _worker_thread_pool;

  int           _srv_num;
  int           _srv_notify[MAX_SERVER_NUM];
  mjtcpsrv      _srv[MAX_SERVER_NUM];
  mjthread      _srv_thread[MAX_SERVER_NUM];
};
typedef struct mjmainsrv* mjmainsrv;

extern bool       mjmainsrv_async(mjtcpsrv srv, mjProc WorkerRoutine, void* w_arg, mjProc CallBack, void* c_arg);
extern bool       mjmainsrv_run(mjmainsrv srv);
extern bool       mjmainsrv_set_stop(mjmainsrv srv, bool value);

extern mjmainsrv  mjmainsrv_new(int sfd, mjProc SrvRoutine, mjProc InitSrv, void* init_arg, int worker_thread_num);
extern bool       mjmainsrv_delete(mjmainsrv srv);

#endif

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
  
  mjthreadpool  _worker_pool;

  int           _srv_num;               // server number
  int           _srv_n[MAX_SERVER_NUM];
  mjtcpsrv      _srv[MAX_SERVER_NUM];   // inner tcp server
  mjthread      _srv_t[MAX_SERVER_NUM]; // inner thread run tcp server
};
typedef struct mjmainsrv* mjmainsrv;

extern bool       mjmainsrv_asy(mjtcpsrv srv, mjProc Routine, void* arg, mjProc CallBack, void* c_arg);
extern bool       mjmainsrv_run(mjmainsrv srv);
extern bool       mjmainsrv_set_stop(mjmainsrv srv, bool value);

extern mjmainsrv  mjmainsrv_new(int sfd, mjProc SrvRoutine);
extern bool       mjmainsrv_delete(mjmainsrv srv);

#endif

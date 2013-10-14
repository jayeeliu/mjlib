#ifndef _MJMAINSRV_H
#define _MJMAINSRV_H

#include <stdbool.h>
#include "mjev.h"
#include "mjthreadpool.h"
#include "mjtcpsrv.h"

// max inner server number
#define MAX_ISNUM  64

struct mjmainsrv {
  int           _sfd;
  bool          _stop;
  
  mjthreadpool  _wpool;           // worker thread pool
  
  unsigned int  _is_num;          // inner server number
  int           _is_n[MAX_ISNUM]; // inner server notify fd
  mjtcpsrv      _is[MAX_ISNUM];   // inner tcp server
  mjthread      _is_t[MAX_ISNUM]; // thread run inner tcp server
};
typedef struct mjmainsrv* mjmainsrv;


extern bool       mjmainsrv_asy(mjtcpsrv msrv, mjProc RT, void* rarg, mjProc CB, void* carg);
extern bool       mjmainsrv_run(mjmainsrv msrv);
extern mjmainsrv  mjmainsrv_new(int sfd, mjProc ISRT);
extern bool       mjmainsrv_delete(mjmainsrv msrv);

static inline mjtcpsrv mjmainsrv_get_is(mjmainsrv msrv, unsigned int index) {
  if (!msrv || index >= msrv->_is_num) return NULL;
  return msrv->_is[index];
}

static inline bool mjmainsrv_set_stop(mjmainsrv msrv, bool value) {
  if (!msrv) return false;
  msrv->_stop = value;
  return true;
}

#endif

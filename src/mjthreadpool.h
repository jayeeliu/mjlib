#ifndef _MJTHREADPOOL_H
#define _MJTHREADPOOL_H

#include "mjthread.h"
#include "mjlockless.h"

struct mjthreadpool {
  int             _nthread;   // thread number
  struct ThrdProc _init;
  bool            _stop;      // stop the thread pool
  bool            _running;   // if this threadpool is running 

  int             _plus;      // plus routine count
  pthread_mutex_t _pluslock;  // plus lock for _plus
  pthread_cond_t  _plusready; // plus ready condition 

  mjlockless      _freelist;
  mjthread        _threads[0];
};
typedef struct mjthreadpool* mjthreadpool;

extern bool         mjthreadpool_set_task(mjthreadpool tpool, mjThrdProc proc, void* arg);
extern bool         mjthreadpool_run(mjthreadpool tpool);

extern mjthreadpool mjthreadpool_new(int nthread);
extern bool         mjthreadpool_delete(mjthreadpool tpool);

static inline bool mjthreadpool_set_init(mjthreadpool tpool, mjThrdProc proc, void* arg) {
  if (!tpool || !proc) return false;
  tpool->_init.proc = proc;
  tpool->_init.arg = arg;
  return true;
}

#endif

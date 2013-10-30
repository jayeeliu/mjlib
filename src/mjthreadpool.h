#ifndef _MJTHREADPOOL_H
#define _MJTHREADPOOL_H

#include "mjthread.h"
#include "mjlockless.h"

struct mjthreadpool {
  int             _nthread;
  mjProc          _INIT;      // thread init routine
  void*           iarg;       // thread init arg
  bool            _stop;      // stop the thread pool
  bool            _running;   // if this threadpool is running 

  int             _plus;      // plus routine count
  pthread_mutex_t _pluslock;  // plus lock for _plus
  pthread_cond_t  _plusready; // plus ready condition 

  mjlockless      _freelist;
  mjthread        _threads[0];
};
typedef struct mjthreadpool* mjthreadpool;


extern bool         mjthreadpool_add_task(mjthreadpool tpool, mjProc RT, void* arg);
extern bool         mjthreadpool_add_task_plus(mjthreadpool tpool, mjProc RT, void* arg);
extern bool         mjthreadpool_run(mjthreadpool tpool);
extern mjthreadpool mjthreadpool_new(int nthread);
extern bool         mjthreadpool_delete(mjthreadpool tpool);


static inline bool mjthreadpool_set_init(mjthreadpool tpool, mjProc INIT, void* iarg) {
  if (!tpool) return false;
  tpool->_INIT = INIT;
  tpool->iarg = iarg;
  for (int i = 0; i < tpool->_nthread; i++) {
    mjthread_set_init(tpool->_threads[i], INIT, iarg);
  }
  return true;
}

#endif

#ifndef _MJTHREADPOOL_H
#define _MJTHREADPOOL_H

#include "mjthread2.h"
#include "mjlockless.h"

// threadpool struct
struct mjthreadpool {
  bool        _stop;         // stop the thread pool
  int         _nthread;
  mjProc      _INIT;
  void*       iarg;
  mjlockless  _free_list;
  mjthread    _threads[0];
};
typedef struct mjthreadpool* mjthreadpool;

extern bool         mjthreadpool_add_routine(mjthreadpool tpool, mjProc RT, void* arg);
extern bool         mjthreadpool_add_routine_plus(mjthreadpool tpool, mjProc RT, void* arg);
extern mjthreadpool mjthreadpool_new(int nthread, mjProc INIT, void* iarg);
extern bool         mjthreadpool_delete(mjthreadpool tpool);

#endif

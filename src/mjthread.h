#ifndef _MJTHREAD_H
#define _MJTHREAD_H

#include "mjproc.h"
#include "mjmap.h"
#include <pthread.h>

struct mjthread;
typedef void* (*mjThrdProc)(struct mjthread*, void*);

struct ThrdProc {
  mjThrdProc  proc;
  void*       arg;
};

struct mjthread {
  pthread_t       _id;        
  pthread_mutex_t _lock;
  pthread_cond_t  _ready;

  struct ThrdProc _init;
  struct ThrdProc _task;
  struct ThrdProc _callback;

  mjmap           _local;   //  local data
  int             _type;    //  thread type
  bool            _stop;    //  true if mjthread_delete has been called
};
typedef struct mjthread* mjthread;


// used for MJTHREAD_NORMAL
extern bool     mjthread_set_task(mjthread thread, mjThrdProc proc, void* arg);
extern bool     mjthread_run(mjthread thread);
// used for MJTHREAD_ONCE
extern bool     mjthread_run_once(mjthread thread, mjThrdProc proc, void* arg);

extern mjthread mjthread_new();
extern bool     mjthread_delete(mjthread thread);

static inline void* mjthread_get_local(mjthread thread, const char* key) {
  if (!thread || !key) return false;
  return mjmap_get_obj(thread->_local, key);
}

static inline bool mjthread_set_local(mjthread thread, const char* key, void* obj, mjProc obj_free) {
  if (!thread || !key) return false;
  if (mjmap_set_obj(thread->_local, key, obj, obj_free) < 0) return false;
  return true;
}

static inline bool mjthread_set_init(mjthread thread, mjThrdProc proc, void* arg) {
  if (!thread) return false;
  thread->_init.proc = proc;
  thread->_init.arg = arg;
  return true;
}

static inline bool mjthread_set_callback(mjthread thread, mjThrdProc proc, void* arg) {
  if (!thread) return false;
  thread->_callback.proc = proc;
  thread->_callback.arg = arg;
  return true;
}

#endif

#ifndef _MJTHREAD_H
#define _MJTHREAD_H

#include "mjproc.h"
#include "mjmap.h"
#include <pthread.h>

struct mjthread {
  pthread_t       _id;        
  pthread_mutex_t _lock;
  pthread_cond_t  _ready;

  mjProc          _INIT;    // run once when thread init, thread routine
  void*           _iarg;    // Init Routine arg
  mjProc          _RT;      // routine to be run, thread routine
  void*           _arg;     // routine arg
  mjProc          _CB;      // CallBack Routine, run when _RT finish, thread routine
  void*           _cbarg;   // CallBack Routine arg

  mjmap           _map;     // arg map for this thread

  int             _type;    // running type
  bool            _stop;    // true if mjthread_delete has been called
};
typedef struct mjthread* mjthread;

// used for MJTHREAD_NORMAL
extern bool     mjthread_add_task(mjthread thread, mjProc RT, void* arg);
extern bool     mjthread_run(mjthread thread);
// used for MJTHREAD_ONCE
extern bool     mjthread_run_once(mjthread thread, mjProc RT, void* arg);

extern mjthread mjthread_new();
extern bool     mjthread_delete(mjthread thread);


static inline void* mjthread_get_arg(mjthread thread) {
  if (!thread) return NULL;
  return thread->_arg;
}

static inline void* mjthread_get_obj(mjthread thread, const char* key) {
  if (!thread || !key) return false;
  return mjmap_get_obj(thread->_map, key);
}

static inline bool mjthread_set_obj(mjthread thread, const char* key, void* obj, mjProc obj_free) {
  if (!thread || !key) return false;
  if (mjmap_set_obj(thread->_map, key, obj, obj_free) < 0) return false;
  return true;
}

static inline void* mjthread_get_iarg(mjthread thread) {
  if (!thread) return NULL;
  return thread->_iarg;
}

static inline bool mjthread_set_init(mjthread thread, mjProc INIT, void* iarg) {
  if (!thread) return false;
  thread->_INIT = INIT;
  thread->_iarg = iarg;
  return true;
}

static inline void* mjthread_get_cbarg(mjthread thread) {
  if (!thread) return NULL;
  return thread->_cbarg;
}

static inline bool mjthread_set_cb(mjthread thread, mjProc CB, void* cbarg) {
  if (!thread) return false;
  thread->_CB = CB;
  thread->_cbarg = cbarg;
  return true;
}

#endif

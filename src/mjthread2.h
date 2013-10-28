#ifndef _MJTHREAD_H
#define _MJTHREAD_H

#include "mjproc.h"
#include "mjmap.h"
#include <pthread.h>

struct mjthread {
  pthread_t       _id;        
  pthread_mutex_t _lock;
  pthread_cond_t  _ready;

  mjProc          _INIT;              // run once when thread init
  void*           iarg;               // Init Routine use this
  mjProc          _RT;                // routine to be run
  void*           arg;                // Routine use this

  mjmap           _map;               // arg map for this thread

  bool            _running;           // true if pthread_create has been called
  bool            _working;           // true if _RT is working 
  bool            _stop;              // 1 when shutdown command has invoked, otherwise 0
};
typedef struct mjthread* mjthread;


extern bool     mjthread_new_once(mjProc INIT, void* iarg, mjProc RT, void* arg);
extern bool     mjthread_add_routine(mjthread thread, mjProc RT, void* arg);
extern bool     mjthread_run(mjthread thread);
extern mjthread mjthread_new();
extern bool     mjthread_delete(mjthread thread);


static inline void* mjthread_get_obj(mjthread thread, const char* key) {
  if (!thread || !key) return false;
  return mjmap_get_obj(thread->_map, key);
}

static inline bool mjthread_set_obj(mjthread thread, const char* key, void* obj, mjProc obj_free) {
  if (!thread || !key) return false;
  if (mjmap_set_obj(thread->_map, key, obj, obj_free) < 0) return false;
  return true;
}

static inline bool mjthread_set_init(mjthread thread, mjProc INIT, void* iarg) {
  if (!thread) return false;
  thread->_INIT = INIT;
  thread->iarg = iarg;
  return true;
}

#endif

#ifndef _MJTHREAD_H
#define _MJTHREAD_H

#include <stdbool.h>
#include <pthread.h>
#include "mjproc.h"
#include "mjmap.h"

struct mjthread {
  pthread_t       _thread_id;
  pthread_mutex_t _thread_lock;
  pthread_cond_t  _thread_ready;

  mjProc          _Init;            // run once when thread init
  void*           init_arg;         // Init Routine use this
  mjProc          _Routine;         // routine to be run
  void*           arg;              // Routine use this

  mjmap           _arg_map;         // arg map for this thread

  bool            _running;
  bool            _closed;          // 1 when thread exit, otherwise 0
  bool            _shutdown;        // 1 when shutdown command has invoked, otherwise 0
};
typedef struct mjthread* mjthread;

extern bool     mjthread_new_once(mjProc Init, void* init_arg, mjProc Routine, void* arg);

extern bool     mjthread_add_routine(mjthread thread, mjProc Routine, void* arg);
extern void*    mjthread_get_obj(mjthread thread, const char* key);
extern bool     mjthread_set_obj(mjthread thread, const char* key, void* obj, mjProc obj_free);

extern mjthread mjthread_new(mjProc Init, void* init_arg);
extern bool     mjthread_delete(mjthread thread);

#endif

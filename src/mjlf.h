#ifndef _MJLF_H
#define _MJLF_H

#include <stdbool.h>
#include "mjthreadpool.h"
#include "mjmap.h"

struct mjlf {
  int           _sfd;     // server socket
  bool          _stop;
  mjthreadpool  _tpool;   // thread pool 
  mjProc        _INIT;    // mjlf server init routine
  void*         iarg;     // mjlf server init arg
  mjProc        _RT;      // run when new conn come
  mjmap         _map;
};
typedef struct mjlf* mjlf;


extern void*  mjlf_run(mjlf srv);
extern mjlf   mjlf_new(int sfd, int max_thread);
extern bool   mjlf_delete(mjlf srv);


static inline void* mjlf_get_obj(mjlf srv, const char* key) {
  if (!srv || !key) return NULL;
  return mjmap_get_obj(srv->_map, key);
}

static inline bool mjlf_set_obj(mjlf srv, const char* key, void* obj, mjProc obj_free) {
  if (!srv || !key) return false;
  if (mjmap_set_obj(srv->_map, key, obj, obj_free) < 0) return false;
  return true;
}

static inline bool mjlf_set_stop(mjlf srv, bool value) {
  if (!srv) return false;
  srv->_stop = value;
  return true;
}

static inline bool mjlf_set_init(mjlf srv, mjProc INIT, void* iarg) {
  if (!srv) return false;
  srv->_INIT = INIT;
  srv->iarg = iarg;
  return true;
}

static inline bool mjlf_set_routine(mjlf srv, mjProc RT) {
  if (!srv) return false;
  srv->_RT = RT;
  return true;
}

static inline bool mjlf_set_thread_init(mjlf srv, mjProc TINIT, void* targ) {
  if (!srv || !srv->_tpool) return false;
  mjthreadpool_set_init(srv->_tpool, TINIT, targ);
  return true;
}

#endif

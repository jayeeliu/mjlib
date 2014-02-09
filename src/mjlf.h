#ifndef _MJLF_H
#define _MJLF_H

#include <stdbool.h>
#include "mjthreadpool.h"
#include "mjconb.h"

struct mjlf;
typedef void* (*mjlfProc)(struct mjlf*, void*);
typedef void* (*mjlfTask)(struct mjlf*, mjthread, mjconb);

struct lfProc {
  mjlfProc  proc;
  void*     arg;
};

struct mjlf {
  int           _sfd;     // server socket
  bool          _stop;
  mjthreadpool  _tpool;   // thread pool 
  struct lfProc _init;
  mjlfTask      _task;    // run when new conn come
  mjmap         _local;
  int           _rto;     // read timeout
  int           _wto;     // write timeout
};
typedef struct mjlf* mjlf;

extern void*  mjlf_run(mjlf srv);
extern mjlf   mjlf_new(int sfd, int max_thread);
extern bool   mjlf_delete(mjlf srv);

static inline bool mjlf_set_timeout(mjlf srv, int rto, int wto) {
  if (!srv) return false;
  srv->_rto = rto;
  srv->_wto = wto;
  return true;
}

static inline void* mjlf_get_local(mjlf srv, const char* key) {
  if (!srv || !key) return NULL;
  return mjmap_get_obj(srv->_local, key);
}

static inline bool mjlf_set_local(mjlf srv, const char* key, void* obj, mjProc obj_free) {
  if (!srv || !key) return false;
  if (mjmap_set_obj(srv->_local, key, obj, obj_free) < 0) return false;
  return true;
}

static inline bool mjlf_set_stop(mjlf srv, bool value) {
  if (!srv) return false;
  srv->_stop = value;
  return true;
}

static inline bool mjlf_set_init(mjlf srv, mjlfProc proc, void* arg) {
  if (!srv || !proc) return false;
  srv->_init.proc = proc;
  srv->_init.arg = arg;
  return true;
}

static inline bool mjlf_set_task(mjlf srv, mjlfTask task) {
  if (!srv || !task) return false;
  srv->_task = task;
  return true;
}

static inline bool mjlf_set_thread_init(mjlf srv, mjThrdProc proc, void* arg) {
  if (!srv || !srv->_tpool || !proc) return false;
  mjthreadpool_set_init(srv->_tpool, proc, arg);
  return true;
}

#endif

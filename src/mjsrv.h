#ifndef _MJTCPSRV_H
#define _MJTCPSRV_H

#include "mjev.h"
#include "mjconn.h"
#include "mjmap.h"
#include "mjthreadpool.h"

struct mjsrv;
typedef void* (*mjsrvProc)(struct mjsrv*, void*);

struct srvProc {
  mjsrvProc proc;
  void*     arg;
};

struct mjsrv {
  int             _sfd;           // socket, accept for standalone, read for inner
  bool            _stop;          // server stop
  mjev            _ev;            // event loop
  struct srvProc  _init;          // init task
  mjProc          _task;          // server task
  mjmap           _local;         // server args map
  mjthreadpool    _tpool;         // threadpool
};
typedef struct mjsrv* mjsrv;

extern bool   mjsrv_enable_listen(mjsrv srv);
extern bool   mjsrv_disable_listen(mjsrv srv);
extern bool   mjsrv_set_tpool(mjsrv srv, int threadNum);
extern bool   mjsrv_async(mjsrv srv, mjProc proc, void* arg, mjProc cbproc, void* cbarg);
extern bool   mjsrv_run(mjsrv srv);

extern mjsrv  mjsrv_new(int sfd);
extern bool   mjsrv_delete(mjsrv srv);

static inline bool mjsrv_set_task(mjsrv srv, mjProc task) {
  if (!srv) return false;
  srv->_task = task;
  return true;
}

static inline bool mjsrv_set_init(mjsrv srv, mjsrvProc proc, void* arg) {
  if (!srv || !proc) return false;
  srv->_init.proc = proc;
  srv->_init.arg = arg;
  return true;
}

static inline void* mjsrv_get_local(mjsrv srv, const char* key) {
  if (!srv || !key) return NULL;
  return mjmap_get_obj(srv->_local, key);
}

static inline bool mjsrv_set_local(mjsrv srv, const char* key, void* obj, mjProc obj_free) {
  if (!srv || !key) return false;
  if (mjmap_set_obj(srv->_local, key, obj, obj_free) < 0) return false;
  return true;
}

static inline bool mjsrv_set_stop(mjsrv srv, bool value) {
  if (!srv) return false;
  srv->_stop = value;
  return true;
}

#endif

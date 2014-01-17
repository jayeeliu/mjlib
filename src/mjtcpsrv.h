#ifndef _MJTCPSRV_H
#define _MJTCPSRV_H

#include "mjev.h"
#include "mjconn.h"
#include "mjmap.h"

#define MJTCPSRV_STANDALONE 0
#define MJTCPSRV_INNER      1

struct mjtcpsrv;
typedef void* (*mjtcpsrvProc)(struct mjtcpsrv*, void*);

struct tcpsrvProc {
  mjtcpsrvProc  proc;
  void*         arg;
};

struct mjtcpsrv {
  int               _sfd;           // socket, accept for standalone, read for inner
  int               _type;          // tcpsrv type, standalone or inner
  bool              _stop;          // server stop
  mjev              _ev;            // event loop
  struct tcpsrvProc _init;          // init task
  mjProc            _task;          // server task
  mjmap             _local;         // server args map
};
typedef struct mjtcpsrv* mjtcpsrv;

extern bool     mjtcpsrv_enable_listen(mjtcpsrv srv);
extern bool     mjtcpsrv_disable_listen(mjtcpsrv srv);
extern void*    mjtcpsrv_run(void *arg);

extern mjtcpsrv mjtcpsrv_new(int sfd, int type);
extern void*    mjtcpsrv_delete(void *arg);

static inline bool mjtcpsrv_set_task(mjtcpsrv srv, mjProc task) {
	if (!srv) return false;
	srv->_task = task;
	return true;
}

static inline bool mjtcpsrv_set_init(mjtcpsrv srv, mjtcpsrvProc proc, void* arg) {
	if (!srv || !proc) return false;
	srv->_init.proc = proc;
	srv->_init.arg = arg;
	return true;
}

static inline mjev mjtcpsrv_get_ev(mjtcpsrv srv) {
  if (!srv) return NULL;
  return srv->_ev;
}

static inline void* mjtcpsrv_get_local(mjtcpsrv srv, const char* key) {
  if (!srv || !key) return NULL;
  return mjmap_get_obj(srv->_local, key);
}

static inline bool mjtcpsrv_set_local(mjtcpsrv srv, const char* key, void* obj, mjProc obj_free) {
  if (!srv || !key) return false;
  if (mjmap_set_obj(srv->_local, key, obj, obj_free) < 0) return false;
  return true;
}

static inline bool mjtcpsrv_set_stop(mjtcpsrv srv, bool value) {
	if (!srv) return false;
	srv->_stop = value;
	return true;
}

#endif

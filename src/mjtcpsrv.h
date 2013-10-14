#ifndef _MJTCPSRV_H
#define _MJTCPSRV_H

#include "mjev.h"
#include "mjmap.h"

#define MJTCPSRV_STANDALONE 0
#define MJTCPSRV_INNER      1

struct mjtcpsrv {
  int     _sfd;   // socket, accept for standalone, read for inner
  int     _type;  // tcpsrv type, standalone or inner
  bool    _stop;  // server stop
  mjev    _ev;    // event loop
  mjProc  _RT;    // server routine
  mjmap   _map;   // server args map
};
typedef struct mjtcpsrv* mjtcpsrv;


extern mjtcpsrv mjtcpsrv_new(int sfd, mjProc RT, int type);
extern void*    mjtcpsrv_delete(void *arg);
extern void*    mjtcpsrv_run(void *arg);


static inline mjev mjtcpsrv_get_ev(mjtcpsrv srv) {
  if (!srv) return NULL;
  return srv->_ev;
}

static inline void* mjtcpsrv_get_obj(mjtcpsrv srv, const char* key) {
  if (!srv || !key) return NULL;
  return mjmap_get_obj(srv->_map, key);
}

static inline bool mjtcpsrv_set_obj(mjtcpsrv srv, const char* key, void* obj,
    mjProc obj_free) {
  if (!srv || !key) return false;
  if (mjmap_set_obj(srv->_map, key, obj, obj_free) < 0) return false;
  return true;
}

static inline bool mjtcpsrv_set_stop(mjtcpsrv srv, bool value) {
	if (!srv) return false;
	srv->_stop = value;
	return true;
}
#endif

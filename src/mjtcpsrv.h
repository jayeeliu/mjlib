#ifndef _MJTCPSRV_H
#define _MJTCPSRV_H

#include "mjev.h"
#include "mjmap.h"

#define MJTCPSRV_STANDALONE 0
#define MJTCPSRV_INNER      1

struct mjtcpsrv {
  int     _sfd;       	// socket, accept for standalone, read for inner
  int     _type;       	// tcpsrv type, standalone or inner
  bool    _stop;       	// server stop
  mjev    ev;           // event loop
  mjProc  _Routine;    	// server routine
  void*   mainSrv;      // used in inner mode, point to mainSrv

  mjProc  _InitSrv;    	// init Server proc
	void*		init_arg;

	mjmap		_arg_map;
};
typedef struct mjtcpsrv* mjtcpsrv;

extern void*	mjtcpsrv_run(void *arg);
extern bool 	mjtcpsrv_set_stop(mjtcpsrv srv, bool value);

extern void*	mjtcpsrv_get_obj(mjtcpsrv srv, const char* key);
extern bool		mjtcpsrv_set_obj(mjtcpsrv srv, const char* key, void* obj, mjProc obj_free);

extern mjtcpsrv mjtcpsrv_new(int sfd, mjProc Routine, mjProc InitSrv, void* init_arg, int type);
extern void*    mjtcpsrv_delete(void *arg);

#endif

#ifndef _MJCONN_H
#define _MJCONN_H

#include "mjstr.h"
#include "mjev.h"
#include "mjthread.h"

struct mjconn {
  int           _fd;                   		// fd 
  mjev          _ev;

  mjstr         _rbuf;                   	// read buffer 
  mjstr         _wbuf;                   	// write buffer
  mjstr         _data;                   	// data get from rbuf

  int           _connect_type;            // connect type
  mjProc        _ConnectCallback;        	// ConnectCallback
  unsigned int  _connect_timeout;        	// connect timeout 
  mjtevent			_connect_timeout_event;  	// connect timeout event

  int           _read_type;              	// readType
  mjProc        _ReadCallBack;           	// read callback 
  char*         _delim;                  	// the delim when readType is READUNTIL
  int           _rbytes;                 	// read data size when readType is READBYTES
  unsigned int  _read_timeout;            // read timeout 
  mjtevent      _read_timeout_event;     	// read timeout event 

  int           _write_type;              // write type 
  mjProc        _WriteCallBack;          	// write callback 
  unsigned int  _write_timeout;          	// write timeout
  mjtevent      _write_timeout_event;    	// write timeout event 

  bool          _error;                  	// some error happened 
  bool          _closed;                 	// fd closed 

	mjmap					_arg_map;
};
typedef struct mjconn* mjconn;

// read func
extern bool mjconn_readbytes(mjconn conn, int len, mjProc CallBack);
extern bool mjconn_readuntil(mjconn conn, char* delim, mjProc CallBack);
extern bool mjconn_read(mjconn conn, mjProc CallBack);
// write func
extern bool mjconn_writes(mjconn conn, char* buf, mjProc CallBack);
extern bool mjconn_write(mjconn conn, mjstr buf, mjProc CallBack);
extern bool mjconn_buf_writes(mjconn conn, char* buf);
extern bool mjconn_buf_write(mjconn conn, mjstr buf);
extern bool mjconn_flush(mjconn conn, mjProc CallBack);
// conn func
extern bool mjconn_connect(mjconn conn, const char* ipaddr, int port, mjProc CallBack);
// timeout func
extern bool mjconn_set_connect_timeout(mjconn conn, unsigned int connect_timeout);
extern bool mjconn_set_timeout(mjconn conn, unsigned int read_timeout, unsigned int write_timeout);
// get or set  objs
extern void*	mjconn_get_obj(mjconn conn, const char* key);
extern bool		mjconn_set_obj(mjconn conn, const char* key, void* obj, mjProc obj_free);

extern mjconn   mjconn_new(mjev ev, int fd);
extern bool     mjconn_delete(mjconn conn);

#endif

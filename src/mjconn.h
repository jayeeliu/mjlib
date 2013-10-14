#ifndef _MJCONN_H
#define _MJCONN_H

#include <string.h>
#include "mjstr.h"
#include "mjev.h"
#include "mjthread.h"

struct mjconn {
  int           _fd;      // fd 
  mjev          _ev;      // event loop

  mjstr         _rbuf;    // read buffer 
  mjstr         _wbuf;    // write buffer
  mjstr         _data;    // data get from rbuf

  int           _ctype;   // connect type
  mjProc        _CCB;     // ConnectCallback
  unsigned int  _cto;     // connect timeout 
  mjtevent      _cto_e;   // connect timeout event

  int           _rtype;   // readType
  mjProc        _RCB;     // read callback 
  char*         _delim;   // the delim when readType is READUNTIL
  int           _rbytes;  // read data size when readType is READBYTES
  unsigned int  _rto;     // read timeout 
  mjtevent      _rto_e;   // read timeout event 

  int           _wtype;   // write type 
  mjProc        _WCB;     // write callback 
  unsigned int  _wto;     // write timeout
  mjtevent      _wto_e;   // write timeout event 

  bool          _error;   // some error happened 
  bool          _closed;  // fd closed 
  bool          _timeout; // conn timeout

  mjmap         _map;
};
typedef struct mjconn* mjconn;


extern bool     mjconn_readbytes(mjconn conn, int len, mjProc CB);
extern bool     mjconn_readuntil(mjconn conn, char* delim, mjProc CB);
extern bool     mjconn_read(mjconn conn, mjProc CallBack);
extern bool     mjconn_writeb(mjconn conn, char* buf, int length, mjProc CB);
extern bool     mjconn_buf_writeb(mjconn conn, char* buf, int length);
extern bool     mjconn_flush(mjconn conn, mjProc CallBack);
extern bool     mjconn_connect(mjconn conn, const char* ipaddr, int port, mjProc CB);
extern mjconn   mjconn_new(mjev ev, int fd);
extern bool     mjconn_delete(mjconn conn);


static inline bool mjconn_buf_write(mjconn conn, mjstr buf) {
  if (!conn || !buf) return false;
  return mjconn_buf_writeb(conn, buf->data, buf->length);
}

static inline bool mjconn_buf_writes(mjconn conn, char* buf) {
  if (!conn || !buf) return false;
  return mjconn_buf_writeb(conn, buf, strlen(buf));
}

static inline bool mjconn_write(mjconn conn, mjstr buf, mjProc CallBack) {
  if (!conn || !buf) return false;
  return mjconn_writeb(conn, buf->data, buf->length, CallBack);
}

static inline bool mjconn_writes(mjconn conn, char* buf, mjProc CallBack) {
  if (!conn || !buf) return false;
  return mjconn_writeb(conn, buf, strlen(buf), CallBack);
}

static inline void* mjconn_get_obj(mjconn conn, const char* key) {
  if (!conn || !key) return NULL;
  return mjmap_get_obj(conn->_map, key);
}

static inline bool mjconn_set_obj(mjconn conn, const char* key, void* obj,
    mjProc obj_free) {
  if (!conn || !key) return false;
  if (mjmap_set_obj(conn->_map, key, obj, obj_free) < 0) return false;
  return true;
}

// set connect timeout
static inline bool mjconn_set_cto(mjconn conn, unsigned int cto) {
  if (!conn) return false;
  conn->_cto = cto;
  return true;
}

// set timeout
static inline bool mjconn_set_to(mjconn conn, unsigned int rto, 
    unsigned int wto) {
  if (!conn) return false;
  conn->_rto = rto;
  conn->_wto = wto;
  return true;
}
#endif

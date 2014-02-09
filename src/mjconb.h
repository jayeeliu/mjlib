#ifndef _MJCONB_H
#define _MJCONB_H

#include "mjstr.h"
#include "mjproc.h"
#include "mjmap.h"
#include <stdbool.h>

#define MJCONB_CLOSED  0
#define MJCONB_ERROR   -1
#define MJCONB_TIMEOUT -2
#define MJCONB_PANIC   -3

struct mjconb {
  int         _fd;      // fd to control
  mjstr       _rbuf;    // read read buffer 
  
  int         _rtype;   // read type
  const char* _delim;   // the delim when readtype is READUNTIL 
  int         _rbytes;  // read data size when readtype is READBYTES 
  
  mjmap       _map;     // arg map

  bool        _closed;  // peer closed
  bool        _error;   // peer error  
  bool        _timeout; // peer timeout
};  
typedef struct mjconb* mjconb;

extern int    mjconb_read(mjconb conn, mjstr buf);
extern int    mjconb_readbytes(mjconb conn, mjstr buf, unsigned int len);
extern int    mjconb_readuntil(mjconb conn, const char* delim, mjstr buf);
extern int    mjconb_write(mjconb conn, mjstr buf);
extern int    mjconb_writeb(mjconb conn, char* buf, unsigned int len);
extern int    mjconb_writes(mjconb conn, char* buf);
extern void*  mjconb_get_obj(mjconb conn, const char* key);
extern bool   mjconb_set_obj(mjconb conn, const char* key, void* obj, mjProc obj_free);
extern bool   mjconb_set_timeout(mjconb conn, unsigned int rto, unsigned int wto);

extern mjconb mjconb_connect(const char* addr, int port, unsigned int timeout);
extern mjconb mjconb_new(int fd);
extern bool   mjconb_delete(mjconb conn);

#endif

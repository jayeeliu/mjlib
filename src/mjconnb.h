#ifndef _MJCONNB_H
#define _MJCONNB_H

#include <stdbool.h>
#include "mjstr.h"
#include "mjproc.h"
#include "mjmap.h"

struct mjconnb {
  int         fd;           // fd to control
  mjstr       rbuf;         // read read buffer 

  int         readtype;     // read type
  const char* delim;        // the delim when readtype is READUNTIL 
  int         rbytes;       // read data size when readtype is READBYTES 

  mjmap       _arg_map;

  bool        timeout;      // peer timeout
  bool        error;        // peer error  
  bool        closed;       // peer closed
};  
typedef struct mjconnb* mjconnb;

extern int    mjconnb_read(mjconnb conn, mjstr data);
extern int    mjconnb_readbytes(mjconnb conn, mjstr data, int len);
extern int    mjconnb_readuntil(mjconnb conn, const char* delim, mjstr data);
extern int    mjconnb_write(mjconnb conn, mjstr data);
extern int    mjconnb_writeb(mjconnb conn, char* buf, int length);
extern int    mjconnb_writes(mjconnb conn, char* buf);

extern void*  mjconnb_get_obj(mjconnb conn, const char* key);
extern bool   mjconnb_set_obj(mjconnb conn, const char* key, void* obj, mjProc obj_free);

extern bool   mjconnb_set_timeout(mjconnb conn, unsigned int read_timeout, 
                unsigned int write_timeout);

extern mjconnb  mjconnb_connect(const char* addr, int port, unsigned int timeout);
extern mjconnb  mjconnb_new(int fd);
extern bool     mjconnb_delete(mjconnb conn);

#endif

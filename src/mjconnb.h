#ifndef _MJCONNB_H
#define _MJCONNB_H

#include <stdbool.h>
#include "mjstr.h"
#include "mjproc.h"

struct mjconnb {
  int         fd;           // fd to control
  mjStr       rbuf;         // read read buffer 

  int         readtype;     // read type
  const char  *delim;       // the delim when readtype is READUNTIL 
  int         rbytes;       // read data size when readtype is READBYTES 

  void        *server;      // server the conn belongs to
  mjProc      FreePrivate;  // free private data callback 
  void        *private;     // private data

  bool        timeout;      // peer timeout
  bool        error;        // peer error  
  bool        closed;       // peer closed
};  
typedef struct mjconnb* mjconnb;

extern int    mjconnb_Read(mjconnb conn, mjStr data);
extern int    mjconnb_ReadBytes(mjconnb conn, mjStr data, int len);
extern int    mjconnb_ReadUntil(mjconnb conn, const char* delim, mjStr data);
extern int    mjconnb_Write(mjconnb conn, mjStr data);
extern int    mjconnb_WriteB(mjconnb conn, char* buf, int length);
extern int    mjconnb_WriteS(mjconnb conn, char* buf);

extern bool   mjconnb_SetPrivate(mjconnb conn, void* private, mjProc FreePrivate);
extern bool   mjconnb_SetServer(mjconnb conn, void* server);
extern bool   mjconnb_SetTimeout(mjconnb conn, unsigned int read_timeout, 
                unsigned int write_timeout);

extern mjconnb  mjconnb_Connect(const char* addr, int port, unsigned int timeout);
extern mjconnb  mjconnb_New(int fd);
extern bool     mjconnb_Delete(mjconnb conn);

#endif

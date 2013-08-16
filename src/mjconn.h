#ifndef _MJCONN_H
#define _MJCONN_H

#include "mjstr.h"
#include "mjev.h"
#include "mjthread.h"

struct mjconn {
  int           fd;                     // fd 
  void*         server;                 // tcpserver for server side conn 
  mjev          ev;

  mjstr         rbuf;                   // read buffer 
  mjstr         wbuf;                   // write buffer
  mjstr         data;                   // data get from rbuf

  int           connect_type;            // connect type
  mjProc        ConnectCallback;        // ConnectCallback
  unsigned int  connect_timeout;         // connect timeout 
  mjtevent*     connect_timeout_event;    // connect timeout event

  int           read_type;               // readType
  mjProc        ReadCallBack;           // read callback 
  char*         delim;                  // the delim when readType is READUNTIL
  int           rbytes;                 // read data size when readType is READBYTES
  unsigned int  read_timeout;            // read timeout 
  mjtevent*     read_timeout_event;       // read timeout event 

  int           write_type;              // write type 
  mjProc        WriteCallBack;          // write callback 
  unsigned int  write_timeout;           // write timeout
  mjtevent*     write_timeout_event;      // write timeout event 

  int           error;                  // some error happened 
  int           closed;                 // fd closed 

  mjProc        FreePrivte;             // free private callback 
  void*         private;                // user conn private data 
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

extern bool mjconn_set_connect_timeout(mjconn conn, unsigned int connect_timeout);
extern bool mjconn_set_timeout(mjconn conn, unsigned int read_timeout, unsigned int write_timeout);
extern bool mjconn_set_private(mjconn conn, void* private, mjProc FreePrivte);
extern bool mjconn_set_server(mjconn conn, void* server);

extern mjconn   mjconn_new(mjev ev, int fd);
extern bool     mjconn_delete(mjconn conn);

#endif

#ifndef _MJCONN_H
#define _MJCONN_H

#include "mjstr.h"
#include "mjev.h"

typedef void* mjthread( void* data );

struct mjconn {
    int     fd;                             // fd 
    void*   server;                         // tcpserver 
    mjev    ev;

    unsigned int    connectTimeout;         // connect timeout 
    unsigned int    readTimeout;            // read timeout 
    unsigned int    writeTimeout;           // write timeout
    mjtevent*       connectTimeoutEvent;    // connect timeout event
    mjtevent*       readTimeoutEvent;       // read timeout event 
    mjtevent*       writeTimeoutEvent;      // write timeout event 

    mjstr rbuf;                             // read buffer 
    mjstr wbuf;                             // write buffer
    mjstr data;                             // data get from rbuf

    int     connectType;                    // connect type
    mjproc* ConnectCallback;                // ConnectCallback

    int     readType;                       // readType
    mjproc* ReadCallBack;                   // read callback 
    char*   delim;                          // the delim when readType is READUNTIL
    int     rbytes;                         // read data size when readType is READBYTES

    int     writeType;                      // write type 
    mjproc* WriteCallBack;                  // write callback 

    int         threadType;                 // thread type
    mjproc*     ThreadCallBack;             // thread callback function 
    int         threadReadNotify;           // thread notify fd 
    int         threadWriteNotify;          // thread notify write fd 
    mjthread*   ThreadRoutine;              // thread routine to be run in runasync

    int error;                              //  some error happened 
    int closed;                             //  fd closed 

    mjproc* FreePrivte;                     // free private callback 
    void*   private;                        //  user conn private data 
};
typedef struct mjconn* mjconn;

// read func
extern bool mjConn_ReadBytes( mjconn conn, int len, mjproc* Proc );
extern bool mjConn_ReadUntil( mjconn conn, char* delim, mjproc* Proc );
extern bool mjConn_Read( mjconn conn, mjproc* Proc );
// write func
extern bool mjConn_WriteS( mjconn conn, char* buf, mjproc* Proc );
extern bool mjConn_Write( mjconn conn, mjstr buf, mjproc* Proc );
extern bool mjConn_BufWriteS( mjconn conn, char* buf );
extern bool mjConn_BufWrite( mjconn conn, mjstr buf );
extern bool mjConn_Flush( mjconn conn, mjproc* Proc );
// thread func
extern bool mjConn_RunAsync( mjconn conn, mjthread* Routine, mjproc* Proc );
// thread func
extern bool mjConn_Connect( mjconn conn, const char* ipaddr, int port, mjproc* proc );

extern bool mjConn_SetConnectTimeout( mjconn conn, unsigned int connectTimeout );
extern bool mjConn_SetTimeout( mjconn conn, unsigned int readTimeout, unsigned int writeTimeout );
extern bool mjConn_SetPrivate( mjconn conn, void* private, mjproc* FreePrivte );

extern mjconn   mjConn_New( void* srv, mjev ev, int fd );
extern void     mjConn_Delete( mjconn conn );

#endif

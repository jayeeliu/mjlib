#ifndef _MJCONN_H
#define _MJCONN_H

#include "mjstr.h"
#include "mjev.h"
#include "mjthread.h"

struct mjConn {
    int     fd;                             // fd 
    void*   server;                         // tcpserver for server side conn 
    mjev    ev;

    unsigned int    connectTimeout;         // connect timeout 
    unsigned int    readTimeout;            // read timeout 
    unsigned int    writeTimeout;           // write timeout
    mjtevent*       connectTimeoutEvent;    // connect timeout event
    mjtevent*       readTimeoutEvent;       // read timeout event 
    mjtevent*       writeTimeoutEvent;      // write timeout event 

    mjStr rbuf;                             // read buffer 
    mjStr wbuf;                             // write buffer
    mjStr data;                             // data get from rbuf

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
typedef struct mjConn* mjConn;

// read func
extern bool mjConn_ReadBytes( mjConn conn, int len, mjproc* Proc );
extern bool mjConn_ReadUntil( mjConn conn, char* delim, mjproc* Proc );
extern bool mjConn_Read( mjConn conn, mjproc* Proc );
// write func
extern bool mjConn_WriteS( mjConn conn, char* buf, mjproc* Proc );
extern bool mjConn_Write( mjConn conn, mjStr buf, mjproc* Proc );
extern bool mjConn_BufWriteS( mjConn conn, char* buf );
extern bool mjConn_BufWrite( mjConn conn, mjStr buf );
extern bool mjConn_Flush( mjConn conn, mjproc* Proc );
// thread func
extern bool mjConn_RunAsync( mjConn conn, mjthread* Routine, mjproc* Proc );
// conn func
extern bool mjConn_Connect( mjConn conn, const char* ipaddr, int port, mjproc* proc );

extern bool mjConn_SetConnectTimeout( mjConn conn, unsigned int connectTimeout );
extern bool mjConn_SetTimeout( mjConn conn, unsigned int readTimeout, unsigned int writeTimeout );
extern bool mjConn_SetPrivate( mjConn conn, void* private, mjproc* FreePrivte );
extern bool mjConn_SetServer( mjConn conn, void* server );

extern mjConn   mjConn_New( mjev ev, int fd );
extern void     mjConn_Delete( mjConn conn );

#endif

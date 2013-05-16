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
    mjProc  ConnectCallback;                // ConnectCallback

    int     readType;                       // readType
    mjProc  ReadCallBack;                   // read callback 
    char*   delim;                          // the delim when readType is READUNTIL
    int     rbytes;                         // read data size when readType is READBYTES

    int     writeType;                      // write type 
    mjProc  WriteCallBack;                  // write callback 

    int         threadType;                 // thread type
    mjProc      ThreadCallBack;             // thread callback function 
    int         threadReadNotify;           // thread notify fd 
    int         threadWriteNotify;          // thread notify write fd 
    mjProc      ThreadRoutine;              // thread routine to be run in runasync

    int error;                              //  some error happened 
    int closed;                             //  fd closed 

    mjProc  FreePrivte;                     // free private callback 
    void*   private;                        //  user conn private data 
};
typedef struct mjConn* mjConn;

// read func
extern bool mjConn_ReadBytes( mjConn conn, int len, mjProc CallBack );
extern bool mjConn_ReadUntil( mjConn conn, char* delim, mjProc CallBack );
extern bool mjConn_Read( mjConn conn, mjProc CallBack );
// write func
extern bool mjConn_WriteS( mjConn conn, char* buf, mjProc CallBack );
extern bool mjConn_Write( mjConn conn, mjStr buf, mjProc CallBack );
extern bool mjConn_BufWriteS( mjConn conn, char* buf );
extern bool mjConn_BufWrite( mjConn conn, mjStr buf );
extern bool mjConn_Flush( mjConn conn, mjProc CallBack );
// thread func
extern bool mjConn_RunAsync( mjConn conn, mjProc Routine, mjProc CallBack );
// conn func
extern bool mjConn_Connect( mjConn conn, const char* ipaddr, int port, mjProc CallBack );

extern bool mjConn_SetConnectTimeout( mjConn conn, unsigned int connectTimeout );
extern bool mjConn_SetTimeout( mjConn conn, unsigned int readTimeout, unsigned int writeTimeout );
extern bool mjConn_SetPrivate( mjConn conn, void* private, mjProc FreePrivte );
extern bool mjConn_SetServer( mjConn conn, void* server );

extern mjConn   mjConn_New( mjev ev, int fd );
extern void     mjConn_Delete( mjConn conn );

#endif

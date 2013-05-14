#ifndef _MJCONN_H
#define _MJCONN_H

#include "mjstr.h"
#include "mjev.h"
#include "mjthread.h"

struct mjConn2 {
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
typedef struct mjConn2* mjConn2;

// read func
extern bool mjConn2_ReadBytes( mjConn2 conn, int len, mjProc CallBack );
extern bool mjConn2_ReadUntil( mjConn2 conn, char* delim, mjProc CallBack );
extern bool mjConn2_Read( mjConn2 conn, mjProc CallBack );
// write func
extern bool mjConn2_WriteS( mjConn2 conn, char* buf, mjProc CallBack );
extern bool mjConn2_Write( mjConn2 conn, mjStr buf, mjProc CallBack );
extern bool mjConn2_BufWriteS( mjConn2 conn, char* buf );
extern bool mjConn2_BufWrite( mjConn2 conn, mjStr buf );
extern bool mjConn2_Flush( mjConn2 conn, mjProc CallBack );
// thread func
extern bool mjConn2_RunAsync( mjConn2 conn, mjProc Routine, mjProc CallBack );
// conn func
extern bool mjConn2_Connect( mjConn2 conn, const char* ipaddr, int port, mjProc CallBack );

extern bool mjConn2_SetConnectTimeout( mjConn2 conn, unsigned int connectTimeout );
extern bool mjConn2_SetTimeout( mjConn2 conn, unsigned int readTimeout, unsigned int writeTimeout );
extern bool mjConn2_SetPrivate( mjConn2 conn, void* private, mjProc FreePrivte );
extern bool mjConn2_SetServer( mjConn2 conn, void* server );

extern mjConn2  mjConn2_New( mjev ev, int fd );
extern bool     mjConn2_Delete( mjConn2 conn );

#endif

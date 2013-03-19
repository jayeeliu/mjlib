#ifndef _MJTCPSRVT_H
#define _MJTCPSRVT_H

#include "mjev.h"
#include "mjthreadpool.h"

struct mjTcpSrvT {
    int             sfd;                // listen fd 
    int             stop;               // stop server
    mjev            ev;    
    mjThreadPool    tpool;              // thread pool 
    mjthread*       Handler;            // worker to run

    mjproc*         InitSrv;            // InitSrv proc
    mjproc*         ExitSrv;            // ExitSrv proc

    void*           private;            // user server private data 
    mjproc*         FreePrivate;        // FreePrivate proc
};
typedef struct mjTcpSrvT* mjTcpSrvT;

extern bool         mjTcpSrvT_Run( mjTcpSrvT srv );
extern bool         mjTcpSrvT_SetHandler( mjTcpSrvT srv, mjthread* Handler );
extern bool         mjTcpSrvT_SetPrivate( mjTcpSrvT srv, void* private, mjproc* FreePrivate );
extern bool         mjTcpSrvT_SetSrvProc( mjTcpSrvT srv, mjproc* InitSrv, mjproc* ExitSrv );
extern bool         mjTcpSrvT_SetStop( mjTcpSrvT srv, int value );

extern mjTcpSrvT    mjTcpSrvT_New( int sfd, int threadNum );
extern bool         mjTcpSrvT_Delete( mjTcpSrvT srv );

#endif
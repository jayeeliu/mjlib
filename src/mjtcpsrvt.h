#ifndef _MJTCPSRVT_H
#define _MJTCPSRVT_H

#include "mjev.h"
#include "mjthreadpool.h"

struct mjTcpSrvT {
    int             sfd;                // listen fd 
    int             stop;               // stop server
    mjev            ev;    
    mjThreadPool    tpool;              // thread pool 
    mjProc          Handler;            // worker to run

    mjProc          InitSrv;            // InitSrv proc
    mjProc          ExitSrv;            // ExitSrv proc

    void*           private;            // user server private data 
    mjProc          FreePrivate;        // FreePrivate proc
};
typedef struct mjTcpSrvT* mjTcpSrvT;

extern bool         mjTcpSrvT_Run( mjTcpSrvT srv );
extern bool         mjTcpSrvT_SetHandler( mjTcpSrvT srv, mjProc Handler );
extern bool         mjTcpSrvT_SetPrivate( mjTcpSrvT srv, void* private, mjProc FreePrivate );
extern bool         mjTcpSrvT_SetSrvProc( mjTcpSrvT srv, mjProc InitSrv, mjProc ExitSrv );
extern bool         mjTcpSrvT_SetStop( mjTcpSrvT srv, int value );

extern mjTcpSrvT    mjTcpSrvT_New( int sfd, int threadNum );
extern bool         mjTcpSrvT_Delete( mjTcpSrvT srv );

#endif

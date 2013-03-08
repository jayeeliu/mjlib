#ifndef _MJTCPSRV_H
#define _MJTCPSRV_H

#include <stdbool.h>
#include "mjev.h"

struct mjTcpSrv {
    int             sfd;
    int             stop;
    mjev            ev;    
    mjproc*         Handler;                // Handler when accept a connect

    mjproc*         InitSrv;                // init Server proc
    mjproc*         ExitSrv;                // exit Server proc

    void*           private;                // user server data, mostly user server struct
    mjproc*         FreePrivate;            // proc used to free private data  
};
typedef struct mjTcpSrv* mjTcpSrv;

extern bool     mjTcpSrv_Run( mjTcpSrv srv );
extern bool     mjTcpSrv_SetHandler( mjTcpSrv srv, mjproc* Handler );
extern bool     mjTcpSrv_SetPrivate( mjTcpSrv srv, void* private, mjproc* FreePrivate );
extern bool     mjTcpSrv_SetSrvProc( mjTcpSrv srv, mjproc* InitSrv, mjproc* ExitSrv );
extern bool     mjTcpSrv_SetStop( mjTcpSrv srv, int value );

extern mjTcpSrv mjTcpSrv_New( int sfd );
extern bool     mjTcpSrv_Delete( mjTcpSrv srv );

#endif

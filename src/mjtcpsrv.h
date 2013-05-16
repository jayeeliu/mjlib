#ifndef _MJTCPSRV_H
#define _MJTCPSRV_H

#include <stdbool.h>
#include "mjev2.h"

struct mjTcpSrv {
    int     sfd;
    int     stop;
    mjEV2   ev;    
    mjProc  Handler;                // Handler when accept a connect

    mjProc  InitSrv;                // init Server proc
    mjProc  ExitSrv;                // exit Server proc

    void*   private;                // user server data, mostly user server struct
    mjProc  FreePrivate;            // proc used to free private data  
};
typedef struct mjTcpSrv* mjTcpSrv;

extern bool     mjTcpSrv_Run( mjTcpSrv srv );
extern bool     mjTcpSrv_SetHandler( mjTcpSrv srv, mjProc Handler );
extern bool     mjTcpSrv_SetPrivate( mjTcpSrv srv, void* private, mjProc FreePrivate );
extern bool     mjTcpSrv_SetSrvProc( mjTcpSrv srv, mjProc InitSrv, mjProc ExitSrv );
extern bool     mjTcpSrv_SetStop( mjTcpSrv srv, int value );

extern mjTcpSrv mjTcpSrv_New( int sfd );
extern bool     mjTcpSrv_Delete( mjTcpSrv srv );

#endif

#ifndef _MJTCPSRV_H
#define _MJTCPSRV_H

#include <stdbool.h>
#include "mjev.h"

struct mjTcpSrvM {
    int             sfd;
    int             stop;
    mjev            ev;    
    mjProc          Handler;                    // Handler when accept a connect

    pthread_t       acceptThread;
    int             acceptThreadNotifyRead;       
    int             acceptThreadNotifyWrite;    // Notifyfd for accept Thread

    mjProc          InitSrv;                    // init Server proc
    mjProc          ExitSrv;                    // exit Server proc

    void*           private;                    // user server data, mostly user server struct
    mjProc          FreePrivate;                // proc used to free private data  
};
typedef struct mjTcpSrvM* mjTcpSrvM;

extern bool         mjTcpSrvM_Run( mjTcpSrvM srv );
extern bool         mjTcpSrvM_SetHandler( mjTcpSrvM srv, mjProc Handler );
extern bool         mjTcpSrvM_SetPrivate( mjTcpSrvM srv, void* private, mjProc FreePrivate );
extern bool         mjTcpSrvM_SetSrvProc( mjTcpSrvM srv, mjProc InitSrv, mjProc ExitSrv );
extern bool         mjTcpSrvM_SetStop( mjTcpSrvM srv, int value );

extern mjTcpSrvM    mjTcpSrvM_New( int sfd );
extern bool         mjTcpSrvM_Delete( mjTcpSrvM srv );

#endif

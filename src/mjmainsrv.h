#ifndef _MJMAINSRV_H
#define _MJMAINSRV_H

#include <stdbool.h>
#include "mjev.h"
#include "mjthreadpool.h"
#include "mjtcpsrv.h"

#define MAX_SERVER_NUM  64

struct mjMainSrv {
    int             sfd;
    int             stop;
    
    int             workerThreadNum;
    mjThreadPool    workerThreadPool;

    mjProc          serverRoutine;
    int             serverNum;
    int             serverNotify[MAX_SERVER_NUM];
    mjTcpSrv        server[MAX_SERVER_NUM];
    mjThread        serverThread[MAX_SERVER_NUM];
};
typedef struct mjMainSrv* mjMainSrv;

extern bool         mjMainSrv_Async( mjMainSrv srv, mjProc Routine, 
                        void* rdata, mjEV ev, mjProc CallBack, void* cdata );
extern bool         mjMainSrv_Run( mjMainSrv srv );
extern mjMainSrv    mjMainSrv_New( int sfd, mjProc serverRoutine, 
                        int workerThreadNum );
extern bool         mjMainSrv_Delete( mjMainSrv srv );

#endif

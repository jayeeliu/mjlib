#define _GNU_SOURCE
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/mman.h>

#include "mjworker.h"
#include "mjlog.h"
#include "mjsock.h"
#include "mjconnb.h"

#define WORKER_FREE     1
#define WORKER_BUSY     2

#define WORKER_RUNNING          10
#define WORKER_SHOULDSTOP       20      //exit normal
#define WORKER_SHOULDRESTART    30      //exit abnormal should restart

#define MAXFD           60000

// used for children
static int* workerStop;
static int* workerStatus;

// used for parent
struct worker_s {
    pid_t   pid;
    int*    stop;
    int*    status;
};
typedef struct worker_s worker_t;
static worker_t* worker = NULL;

// map from pid to procid
static int pidToProcID[MAXFD] = { -1 };

// alarm process exit from accept
static void WorkerAlarm( int sig ) { }

/*
=========================================================================
WorkerNew
    fork new worker, share status and stop with parent
    return -1    ---- fork error
            0    ---- child return
            1    ---- parent return
=========================================================================
*/
static int WorkerNew( int procID )
{
    // map status and stop
    int fd = open( "/dev/zero", O_RDWR );
    if ( fd < 0 ) {
        MJLOG_ERR( "open /dev/zero error" );
        return -1;
    }
    workerStatus = ( int* ) mmap( 0, sizeof( int ), PROT_READ | PROT_WRITE,
                                    MAP_SHARED, fd, 0 );
    if ( workerStatus == MAP_FAILED ) {
        MJLOG_ERR( "mmap failed" );
        close( fd );
        return -1;
    }
    workerStop  = ( int* ) mmap( 0, sizeof( int ), PROT_READ | PROT_WRITE,
                                    MAP_SHARED, fd, 0 );
    if ( workerStop == MAP_FAILED ) {
        MJLOG_ERR( "mmap failed" );
        close( fd );
        return -1;
    }
    close( fd );


    pid_t pid = fork();
    // for child
    if ( !pid ) {
        // set SIGUSER1 signal
        struct sigaction act;
        act.sa_handler = WorkerAlarm;
        sigemptyset( &act.sa_mask );
        act.sa_flags = 0;
        sigaction( SIGUSR1, &act, NULL);
        // set worker status
        *( workerStatus )   = WORKER_FREE;
        *( workerStop )     = WORKER_RUNNING;
        // free worker
        free( worker );
        worker = NULL;
        return 0;
        //TODO: memory leak here, parent map area for other process
    }
    // fork error 
    if ( pid < 0 ) {
        munmap( workerStatus, sizeof( int ) );
        munmap( workerStop, sizeof( int ) );
        return -1;
    }
    // for parent, status stop have been mapped
    // if it is restart clean it first. 
    if ( worker[procID].status ) { 
        munmap( worker[procID].status, sizeof( int ) );
    }
    if ( worker[procID].stop ) { 
        munmap( worker[procID].stop, sizeof( int ) );
    }
    // set worker array
    worker[procID].pid      =   pid;
    worker[procID].status   =   workerStatus;
    worker[procID].stop     =   workerStop;
    pidToProcID[pid]        =   procID;
    workerStatus            =   NULL;
    workerStop              =   NULL;
  
    return 1;
}

/*
==============================================================
WorkerChild
    SIG_CHLD handler, restart worker if exit abnormally.
==============================================================
*/
static void WorkerChild( int sig )
{
    int     status;
    pid_t   pid;

    while ( ( pid = waitpid( -1, &status, WNOHANG) ) > 0 ) {
        int procID = pidToProcID[pid];
        if ( worker[procID].pid != pid ) {
            MJLOG_ERR( "Oops!!! pid is not equal" );
            continue;
        }
        // for abnormal stop, set it WORKER_SHOULDRESTART
        if ( *( worker[procID].stop ) != WORKER_SHOULDSTOP ) {
            MJLOG_WARNING( "proc exit abnormal restart it" );
            *( worker[procID].stop ) = WORKER_SHOULDRESTART;
            continue;
        }
        // for normal stop, parent unmap status and stop
        munmap( worker[procID].status, sizeof( int ) );
        munmap( worker[procID].stop, sizeof( int ) );
        worker[procID].status   =   NULL;
        worker[procID].stop     =   NULL;
    }
}

/*
===================================================
WorkerSpawn
    fork procs workers.
    parent never return
    child return 
===================================================
*/
static void WorkerSpawn( int minProcs, int maxProcs ) 
{
    // alloc worker array
    worker = ( worker_t* ) calloc( maxProcs, sizeof( worker_t ) );
    if ( !worker ) {
        MJLOG_ERR( "worker_t call error" );
        goto out;
    }

    int epfd = epoll_create( 1024 );
    if ( epfd < 0 ) {
        MJLOG_ERR( "epoll_create error" );
        free(worker);
        goto out;
    }
    // handle signal
    signal( SIGCHLD, WorkerChild );
   
    int procNum = minProcs;
    for ( int i = 0; i < procNum; ++i ) {
        // fork, child return 
        if ( !WorkerNew( i ) ) return;
    }
  
    // parent, run 
    int checkShink = 0;
    struct epoll_event epevents[MAXFD];
    while ( 1 ) {
        int numevents = epoll_wait( epfd, epevents, procNum, 1000 );
        if ( numevents < 0 ) {
            if ( errno == EINTR ) continue;
            MJLOG_ERR( "epoll_wait error" );
            break;
        }
        
        // check abnormal exit process
        for ( int i = 0; i < procNum; i++ ) {
            if ( *( worker[i].stop ) == WORKER_SHOULDRESTART ) {
                if ( !WorkerNew( i ) ) return;
            }
        }

        // add or free process
        int freeCnt = 0;
        for ( int i = 0; i < procNum; i++ ) {
            if ( *( worker[i].status ) == WORKER_FREE ) freeCnt++;
        }

        MJLOG_WARNING( "freeCnt: %d, procNum: %d", freeCnt, procNum );
        if ( freeCnt <=  2 && procNum < maxProcs ) { // expand process 
            MJLOG_WARNING( "fork new worker %d", procNum );
            if ( !WorkerNew( procNum ) ) return;
            procNum++;
        } else if ( ( freeCnt * 3 / procNum ) >= 2 && procNum > minProcs ) {
            if ( ++checkShink < 5 ) continue; // check 5 times then shrink process
            checkShink = 0;
            procNum--;
            *( worker[procNum].stop ) = WORKER_SHOULDSTOP;            
            MJLOG_WARNING( "stop process %d", procNum );
            if ( kill( worker[procNum].pid, SIGUSR1 ) < 0 ) {
                MJLOG_ERR ( "notify Stop error: %s", strerror(errno) );
                *( worker[procNum].stop ) = WORKER_RUNNING;
                procNum++;
            }
            continue;
        }
        checkShink = 0;

    }

out:
    MJLOG_ERR( "Main Process Exit" );
    exit(1);
}

/*
=====================================================================
WorkerRun
    prefork server, and run proc
=====================================================================
*/
int WorkerRun( int minProcs, int maxProcs, int sfd, workproc* proc )
{
    if ( minProcs > maxProcs || minProcs <= 0 || maxProcs <= 0 ) {
        MJLOG_ERR( "minProcs maxProcs check error" );
        return -1;
    }
    if ( !proc ) {
        MJLOG_ERR( "proc is null" );
        return -1;
    }

    // prefork, parent never return
    WorkerSpawn( minProcs, maxProcs );
    // worker run 
    while ( *( workerStop ) != WORKER_SHOULDSTOP ) {
        *( workerStatus ) = WORKER_FREE;
        int cfd = mjSock_Accept( sfd );
        if ( cfd < 0 ) {
            if ( errno == EINTR ) continue;
            MJLOG_ERR( "Oops accept error exit" );
            break;
        }
        *( workerStatus ) = WORKER_BUSY;

        mjConnB conn = mjConnB_New( cfd );
        if ( !conn ) {
            MJLOG_ERR( "mjConnB create error" );
            close( cfd );
            continue;
        }
      
        // run process 
        proc( conn ); 
        // destroy conn
        mjConnB_Delete( conn );
    }
    mjSock_Close( sfd );

    return 0;
}

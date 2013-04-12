#include <stdio.h>
#include <unistd.h>

#include <mjsock.h>
#include <mjthread.h>
#include <mjev.h>

#define WORKER_NUM 4

struct LoopServer {
    mjev        ev;
    int         nfd[2];
    mjThread    thread;
};
typedef struct LoopServer* LoopServer;


void* MyWorker( void* arg )
{
    LoopServer server = ( LoopServer ) arg;

    mjEV_Run( server->ev );

    return NULL;
}

void AcceptHandler( void* arg )
{
    LoopServer server = ( LoopServer ) arg;
    int cfd;
    read( server->nfd[0], &cfd, sizeof( int ) );
    printf( "read socket %d, in thread: %ld\n", cfd, server->thread->threadID );
    close( cfd );
}

int main()
{
    struct LoopServer server[WORKER_NUM];

    for ( int i = 0; i < WORKER_NUM; ++i ) {
        server[i].ev = mjEV_New();
        pipe( server[i].nfd );
        mjEV_Add( server[i].ev, server[i].nfd[0], MJEV_READABLE, AcceptHandler, &server[i] );
        server[i].thread = mjThread_NewLoop( MyWorker, &server[i] );
    }

    int sfd = mjSock_TcpServer( 7879 );
    int balance = 0;
    while ( 1 ) {
        int cfd = mjSock_Accept( sfd );
        write( server[balance].nfd[1], &cfd, sizeof( int ) );
        balance = ( balance + 1 ) % WORKER_NUM;
    }

    for ( int i = 0; i < WORKER_NUM; ++i ) {
        mjThread_Delete( server[i].thread );
    }

    return 0;
}

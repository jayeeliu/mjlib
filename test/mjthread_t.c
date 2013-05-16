#include <stdio.h>
#include <unistd.h>

#include <mjsock.h>
#include <mjthread.h>
#include <mjev2.h>
#include <mjconn2.h>

#define WORKER_NUM 2

struct LoopServer {
    mjEV2        ev;
    int         nfd[2];
    mjThread    thread;
};
typedef struct LoopServer* LoopServer;

void* on_close( void* arg )
{
    mjConn2 conn = ( mjConn2 ) arg;
    mjConn2_Delete( conn );
    return NULL;
}

void* on_read( void* arg )
{
    mjConn2 conn = ( mjConn2 ) arg;
    mjConn2_WriteS( conn, "read OK!", on_close );
    return NULL;
}

void* AcceptHandler( void* arg )
{
    LoopServer server = ( LoopServer ) arg;
    int cfd;
    read( server->nfd[0], &cfd, sizeof( int ) );
    mjConn2 conn = mjConn2_New( server->ev, cfd );

    mjConn2_ReadUntil( conn, "\r\n\r\n", on_read );
    return NULL;
}

void* MyWorker( void* arg )
{
    LoopServer server = ( LoopServer ) arg;
    
    mjEV2_Run( server->ev );
    return NULL;
}

int main()
{
    struct LoopServer server[WORKER_NUM];

    for ( int i = 0; i < WORKER_NUM; ++i ) {
        server[i].ev = mjEV2_New();
        pipe( server[i].nfd );
        mjEV2_Add( server[i].ev, server[i].nfd[0], MJEV_READABLE, AcceptHandler, &server[i] );
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

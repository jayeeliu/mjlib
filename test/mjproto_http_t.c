#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include "mjconn.h"
#include "mjhttprsp.h"
#include "mjproto_http.h"
#include "mjtcpsrv.h"
#include "mjsock.h"

static int count = 0;

struct timeval tv1, tv2;

static void* on_finish(void *arg)
{
    mjConn conn = (mjConn)arg;
    //if ( count >= 99999 ) mjTcpSrv_SetStop( conn->server, 1);
    mjConn_Delete(conn);
    
    gettimeofday( &tv2, NULL );
//    printf("%ld\n", (tv2.tv_usec - tv1.tv_usec) );
    count++;
    return NULL;
}

static void* main0(void *arg)
{
    mjConn conn = (mjConn)arg;
    mjConn_WriteS(conn, "main0 is hereaslfkjlaskfjlkasfdj;aslkdjf;asldjf;aslkjfd;asldkfj;aslkdjf;alsdkjf;aslkdjfas;ldkfjas;dlkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkf", on_finish);
    return NULL;
}

static void* main1(void *arg)
{
    gettimeofday( &tv1, NULL );
    mjConn conn = (mjConn)arg;
    mjConn_WriteS(conn, "main1 is hereaslfkjlaskfjlkasfdj;aslkdjf;asldjf;aslkjfd;asldkfj;aslkdjf;alsdkjf;aslkdjfas;ldkfjas;dlkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkf", on_finish);
    return NULL;
}

static void* main2(void *arg)
{
    mjConn conn = (mjConn)arg;

    gettimeofday( &tv1, NULL );

    mjStrList strList = HTTP_GETPARAM( conn );
    mjStr strtmp = mjStrList_Get( strList , 1 );

    char buf[1024] = { 0 };
    char buflen[128] = { 0 };
    sprintf(buf, "这是一个测试，程序输出: %s", strtmp->data );
    sprintf(buflen, "%d", (int)strlen(buf));

    mjHttpRsp response = HTTP_GETRSP( conn );
    mjHttpRsp_AddHeader( response, "Content-Length", buflen );

    mjConn_BufWriteS( conn , "HTTP/1.1 200 OK\r\n" );
    mjStr str = mjHttpRsp_HeaderToStr( response );
    mjConn_BufWrite( conn, str );
    mjConn_BufWriteS( conn, "\r\n" );
    mjConn_WriteS(conn, buf, on_finish);
    mjStr_Delete( str );
    return NULL; 
}

static void* main3(void *arg)
{
    mjConn conn = (mjConn)arg;

    mjStr out = FileToStr("test.html");

    char buf[1024] = { 0 };
    sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n", out->length);

    mjConn_BufWriteS( conn, buf );
    mjConn_BufWrite( conn, out );
    mjConn_Flush(conn, on_finish);

    mjStr_Delete(out);
    return NULL;
}

struct mjHttpUrl urls[] = {
    {"^/1/$",  main1, NULL},
    {"^/2([0-9]*)/$",  main2, NULL},
    {"^/3/$", main3, NULL},
    {"^/$",   main0, NULL},
    {NULL,  on_finish, NULL},
};

int main()
{
    int sfd = mjSock_TcpServer( 7879 );
    if ( sfd < 0 ) {
        printf( "Error create server socket\n" );
        return 1;
    }

    mjTcpSrv server = mjTcpSrv_New( sfd );
    if ( !server ) {
        printf( "Error create tcpserver\n" );
        return 1;
    }

    mjTcpSrv_SetPrivate( server, urls, NULL ); 
    mjTcpSrv_SetSrvProc( server, http_InitSrv, http_ExitSrv );
    mjTcpSrv_SetHandler( server, http_Worker );
    mjTcpSrv_Run( server );

    mjTcpSrv_Delete( server );
    return 0;
}

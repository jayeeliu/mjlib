#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "mjtcpsrv.h"
#include "mjlog.h"
#include "mjconn.h"
#include "mjproto_http.h"

static void on_header( void* arg )
{
    mjConn conn = ( mjConn )arg;
    mjHttpData httpData = ( mjHttpData )conn->private;
    mjTcpSrv server = ( mjTcpSrv )conn->server;
    struct mjHttpUrl* urls = ( struct mjHttpUrl* ) server->private;
   
    //alloc request and response struct 
    httpData->request = mjHttpReq_New( conn->data );
    if ( !httpData->request ) {
        MJLOG_ERR( "mjHttpReq_New error" );
        mjConn_Delete( conn );
        return;
    }
    httpData->response = mjHttpRsp_New();
    if ( !httpData->response ) {
        MJLOG_ERR( "mjHttpRsp_new error" );
        mjConn_Delete( conn );
        return;
    }
    httpData->param = mjStrList_New();
    if ( !httpData->param ) {
        MJLOG_ERR( "mjStrList_New error" );
        mjConn_Delete( conn );
        return;
    }
    // set default response header
    mjHttpRsp_AddHeader( httpData->response, "Content-Type", 
                    "text/html; charset=UTF-8" );
    mjHttpRsp_AddHeader( httpData->response, "Server", "SFQ-0.01" );
    // call function
    mjStr location = httpData->request->location;
    if ( location->data[location->length - 1] != '/' ) {
        mjStr_CatS( location, "/" );
    }
    // check string match
    int i;
    for ( i = 0; urls[i].url != NULL; i++ ) {
        if ( mjReg_Search( urls[i].reg, location->data, httpData->param ) ) {
            urls[i].fun( conn );
            return;
        }
    }
    urls[i].fun( conn );
    return;
}

/*
================================================
httpData_free
    free per conn data
================================================
*/
static void httpData_free( void* arg )
{
    mjHttpData httpData = ( mjHttpData )arg;
    if ( httpData->request ) {
        mjHttpReq_Delete( httpData->request );
    }
    if ( httpData->response ) {
        mjHttpRsp_Delete( httpData->response );
    }
    if ( httpData->param ) {
        mjStrList_Delete( httpData->param );
    }
    free( httpData );
}

/*
==========================================================
http_worker
    main worker of http protocol
    alloc per conn data, store in private
    read header
==========================================================
*/
void http_Worker( void* arg )
{
    mjConn conn = ( mjConn )arg;
    void* httpData = calloc( 1, sizeof( struct mjHttpData ) );
    if ( !httpData ) {
        MJLOG_ERR( "httpData alloc error" );
        mjConn_Delete( conn );
        return;
    }
    // set conn private data
    mjConn_SetPrivate( conn, httpData, httpData_free );
    mjConn_ReadUntil( conn, "\r\n\r\n", on_header ); 
}

/*
==============================================
http_InitSrv
    called when httpserver run
    urls must be set first
    create mjreg
==============================================
*/
void http_InitSrv( void* arg )
{
    mjTcpSrv srv = ( mjTcpSrv )arg;
    struct mjHttpUrl* urls = srv->private;
    if ( !urls ) {
        MJLOG_ERR( "Oops urls is null" );
        return;
    }
    // create mjreg
    for ( int i = 0; urls[i].url != NULL; i++ ) {
        urls[i].reg = mjReg_New( urls[i].url );
        if ( !urls[i].reg ) {
            MJLOG_ERR( "mjreg_new Error" );
            return;
        }
    }
}

/*
===============================================
http_ExitSrv
    called when httpserver exit
    delete mjreg
===============================================
*/
void http_ExitSrv( void* arg )
{
    mjTcpSrv srv = ( mjTcpSrv )arg;
    struct mjHttpUrl* urls = srv->private;
    if ( !urls ) {
        MJLOG_ERR( "Oops urls is null" );
        return;
    }
    // delete mjReg
    for ( int i = 0; urls[i].url != NULL; i++ ) {
        if ( !urls[i].reg ) {
            mjReg_Delete( urls[i].reg );
        } 
    }
}

/*
===========================================
FileToStr
    read file and store data into mjStr
===========================================
*/
mjStr FileToStr( const char* fileName )
{
    mjStr out = mjStr_New();
    if ( !out ) {
        MJLOG_ERR( "mjStr alloc error" );
        return NULL;
    }

    int fd = open( fileName, O_RDONLY );
    if ( fd < 0 ) {
        MJLOG_ERR( "open file error" );
        mjStr_Delete( out );
        return NULL;
    }

    char buf[1024];
    while ( 1 ) {
        int ret = read( fd, buf, sizeof( buf ) );
        if ( ret <= 0 ) break;
        mjStr_CatB( out, buf, ret );
    }
    close( fd );

    return out;
}

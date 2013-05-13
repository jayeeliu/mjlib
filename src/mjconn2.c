#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "mjconn.h"
#include "mjlog.h"
#include "mjsock.h"

#define BUF_SIZE    4096

#define MJCONN_NONE         0
#define MJCONN_READBYTES    1
#define MJCONN_READUNTIL    2
#define MJCONN_READ         3
#define MJCONN_WRITE        10
#define MJCONN_THREAD       100
#define MJCONN_CONN         1000

/*
=================================================
mjConn_TimeoutCallBack
    read/write timeout callback
=================================================
*/
static void* mjConn_TimeoutCallBack( void* data )
{
    mjConn conn = ( mjConn ) data;
    MJLOG_ERR( "timeout" );
    mjConn_Delete( conn );
    return NULL;
}

/*
====================================================
mjConn_DelReadEvent
    read finish del event and run callback
====================================================
*/
static void* mjConn_DelReadEvent( mjConn conn )
{
    // del read event
    mjEV_Del( conn->ev, conn->fd, MJEV_READABLE );
    if ( conn->readTimeout ) {
        mjEV_DelTimer( conn->ev, conn->readTimeoutEvent ); // invalid timer event
        conn->readTimeout       = 0;
        conn->readTimeoutEvent  = NULL;
    }
    // reset read type
    conn->readType = MJCONN_NONE;
    // run callback in the last step
    if ( conn->ReadCallBack ) conn->ReadCallBack( conn );
    return NULL;
}

/*
=======================================================
mjConn_ReadEventCallBack
    run when data come
    when error or closed, close conn
=======================================================
*/
static void* mjConn_ReadEventCallBack( void* arg )
{
    mjConn conn = ( mjConn )arg;
    char buf[BUF_SIZE];

	// read data first in a loop
    for ( ;; ) {
        int ret = read( conn->fd, buf, BUF_SIZE );
        if ( ret == -1 ) {                  // read data error
            if ( errno == EINTR ) continue; // interupt by signal, continue
            if ( errno == EAGAIN ) break;   // nonblock, no data, break
            conn->error = 1;                // error happens
            MJLOG_ERR( "read error" );
            break;
        }
        if ( ret == 0 ) {       // read data finish 
            conn->closed = 1;   // read port is closed
            MJLOG_ERR( "conn is closed" );
            break;
        }
        // we read some data, put it to rbuf
        mjStr_CatB( conn->rbuf, buf, ret );
        break;
    }

    if ( conn->readType == MJCONN_READBYTES ) { // read type is readbytes
        if ( conn->rbytes <= conn->rbuf->length ) { 
            mjStr_CopyB( conn->data, conn->rbuf->data, conn->rbytes );
            mjStr_Consume( conn->rbuf, conn->rbytes );
            mjConn_DelReadEvent( conn );
            return NULL;
        }
    } else if ( conn->readType == MJCONN_READUNTIL ) { // read type is readuntil
        int pos = mjStr_Search( conn->rbuf, conn->delim );
        if ( pos != -1 ) {
            mjStr_CopyB( conn->data, conn->rbuf->data, pos );
            mjStr_Consume( conn->rbuf, pos + strlen( conn->delim ) );
            mjConn_DelReadEvent( conn );
            return NULL;
        }
    } else if ( conn->readType == MJCONN_READ ) { // read type is normal read
        if ( conn->rbuf && conn->rbuf->length > 0 ) {
            mjStr_CopyB( conn->data, conn->rbuf->data, conn->rbuf->length );
            mjStr_Consume( conn->rbuf, conn->rbuf->length );
            mjConn_DelReadEvent( conn );
            return NULL;
        }
    }
    // some error happend, close conn
    if ( conn->closed || conn->error ) {
        mjConn_Delete( conn );
    }
    return NULL;
}

/*
==============================================
mjConn_AddReadEvent
    add read event, read timeout event
    return false -- add error, close conn
            true -- add success
==============================================
*/
static bool mjConn_AddReadEvent( mjConn conn )
{
    // check if conn has closed or error
    if ( conn->closed || conn->error ) {
        MJLOG_ERR( "conn is closed or error" );
        goto failout;
    }
    // add readevent
    int ret = mjEV_Add( conn->ev, conn->fd, MJEV_READABLE, mjConn_ReadEventCallBack, conn );
    if ( ret < 0 ) {
        MJLOG_ERR( "mjEV_Add error" );
        goto failout;
    }
    // add read timeout event
    if ( conn->readTimeout ) {
        conn->readTimeoutEvent = 
            mjEV_AddTimer( conn->ev, conn->readTimeout, mjConn_TimeoutCallBack, conn );
        if ( !conn->readTimeoutEvent ) {
            MJLOG_ERR( "mjEV_AddTimer error" );
            goto failout;
        }
    }
    return true;

failout:
    mjConn_Delete( conn );
    return false;
}

/*
=========================================================
mjConn_ReadBytes
    read len bytes
    return false -- error, true -- success
=========================================================
*/
bool mjConn_ReadBytes( mjConn conn, int len, mjProc CallBack )
{
    // sanity check
    if ( !conn || !CallBack ) {
        MJLOG_ERR( "conn or CallBack is null" );  
        return false;
    }
    // can't re enter
    if ( conn->readType != MJCONN_NONE ) {
        MJLOG_ERR( "readType must be MJCONN_NONE" );
        return false;
    }
    // set read type
    conn->readType      = MJCONN_READBYTES;        
    conn->rbytes        = len;
    conn->ReadCallBack  = CallBack;
    // check rbuf
    if ( conn->rbytes <= conn->rbuf->length ) { 
        // copy rbytes to data
        mjStr_CopyB( conn->data, conn->rbuf->data, conn->rbytes );
        mjStr_Consume( conn->rbuf, conn->rbytes );
        // read finish
        conn->readType = MJCONN_NONE;
        // run callback
        if ( conn->ReadCallBack ) conn->ReadCallBack( conn );
        return true;
    }
    // add to event loop
    return mjConn_AddReadEvent( conn );
}

/*
==============================================================
mjConn_ReadUntil
    mjConn read until delim 
    return false --- error, true -- readfinish or set event ok
==============================================================
*/
bool mjConn_ReadUntil( mjConn conn, char* delim, mjProc CallBack )
{
    // sanity check
    if ( !conn || !delim || !CallBack ) {
        MJLOG_ERR( "conn or delim or proc is null" );
        return false;
    }
    // can't re enter
    if ( conn->readType != MJCONN_NONE ) {
        MJLOG_ERR("readType must be MJCONN_NONE");
        return false;
    }
    // set read type
    conn->readType      = MJCONN_READUNTIL;
    conn->delim         = delim;
    conn->ReadCallBack  = CallBack;
    // found data in rbuf, call proc and return 
    int pos = mjStr_Search( conn->rbuf, conn->delim );
    if ( pos != -1 ) {
        // copy data to rbuf, not include delim
        mjStr_CopyB( conn->data, conn->rbuf->data, pos );
        mjStr_Consume( conn->rbuf, pos + strlen( conn->delim ) );
        // read finish set readType to NONE, run callback
        conn->readType = MJCONN_NONE;
        // callback must be the last statement, callback maybe run another read or write
        if ( conn->ReadCallBack ) conn->ReadCallBack( conn );
        return true;
    }
    // add read event to event loop 
    return mjConn_AddReadEvent( conn ); 
}

/*
===========================================================
mjConn_Read
    read data
===========================================================
*/
bool mjConn_Read( mjConn conn, mjProc CallBack )
{
    // sanity check
    if ( !conn || !CallBack ) {
        MJLOG_ERR( "conn or CallBack is null" );
        return false;
    }
    // can't re enter
    if ( conn->readType != MJCONN_NONE ) {
        MJLOG_ERR( "readType must be MJCONN_NONE" );
        return false;
    }
    // set read type
    conn->readType      = MJCONN_READ;
    conn->ReadCallBack  = CallBack;
    // found data in rbuf
    if ( conn->rbuf && conn->rbuf->length > 0 ) {
        mjStr_CopyB( conn->data, conn->rbuf->data, conn->rbuf->length );
        mjStr_Consume( conn->rbuf, conn->rbuf->length );
        conn->readType  = MJCONN_NONE;
        if ( conn->ReadCallBack ) conn->ReadCallBack( conn );
        return 0;
    }
    return mjConn_AddReadEvent( conn );
}

/*
==========================================================
mjConn_DelWriteEvent
    del write event
==========================================================
*/
static void* mjConn_DelWriteEvent( mjConn conn )
{
    mjEV_Del( conn->ev, conn->fd, MJEV_WRITEABLE );
    // del write timeout event
    if ( conn->writeTimeout ) {
        mjEV_DelTimer( conn->ev, conn->writeTimeoutEvent );
        conn->writeTimeout      = 0;    
        conn->writeTimeoutEvent = NULL;
    }
    // set write type to NONE
    conn->writeType = MJCONN_NONE;
    // call write callback
    if ( conn->WriteCallBack ) conn->WriteCallBack( conn );
    return NULL;
}

/*
===========================================================
mjConn_WriteEventCallback
    run when we can write data
===========================================================
*/
static void* mjConn_WriteEventCallback( void* arg)
{
    mjConn conn = ( mjConn )arg;
    int ret = write( conn->fd, conn->wbuf->data, conn->wbuf->length );
    if ( ret < 0 ) {
        MJLOG_ERR( "conn write error: %s", strerror( errno ) );
        mjConn_Delete( conn );
        return NULL;
    }
    mjStr_Consume( conn->wbuf, ret );
    // no data to write call DelWriteEvent
    if ( conn->wbuf->length == 0 ) {
        mjConn_DelWriteEvent( conn );
    }
    return NULL;
}

/*
==================================================
mjConn_AddWriteEvent
    add write event to eventloop
==================================================
*/
static bool mjConn_AddWriteEvent( mjConn conn )
{
    // sanity check
    if ( conn->closed || conn->error ) {
        MJLOG_ERR( "conn is closed or error" );
        goto failout;
    }
    // add write event
    int ret = mjEV_Add( conn->ev, conn->fd, MJEV_WRITEABLE, mjConn_WriteEventCallback, conn );
    if ( ret < 0 ) {
        MJLOG_ERR( "mjEV_Add error" );
        goto failout;
    }
    // AddWriteEvent can be call many times
    // When we call it twice, we can't change the callback
    if ( conn->writeTimeout && !conn->writeTimeoutEvent ) { 
        conn->writeTimeoutEvent = 
                mjEV_AddTimer( conn->ev, conn->writeTimeout, mjConn_TimeoutCallBack, conn );
        if ( !conn->writeTimeoutEvent ) {
            MJLOG_ERR( "mjEV_AddTimer error" );
            goto failout;
        }
    }
    return true;

failout:
    mjConn_Delete( conn );
    return false;
}

/*
===============================================
mjConn_BufWriteS
    copy string to wbuf
===============================================
*/
bool mjConn_BufWriteS( mjConn conn, char* buf)
{
    if ( !conn || !buf ) {
        MJLOG_ERR( "conn or buf is null" );
        return false;
    }
    mjStr_CatS( conn->wbuf, buf );
    return true;
}

/*
==============================================
mjConn_BufWrite
    copy mjStr to wbuf
==============================================
*/
bool mjConn_BufWrite( mjConn conn, mjStr buf )
{
    if ( !conn || !buf ) {
        MJLOG_ERR( "conn or buf is null" );
        return false;
    }
    return mjConn_BufWriteS( conn, buf->data );
}

/*
===============================================
mjConn_Flush
    flush wbuf
===============================================
*/
bool mjConn_Flush( mjConn conn, mjProc CallBack )
{
    // sanity check
    if ( !conn ) {
        MJLOG_ERR("conn is null");
        return false;
    }
    // set write callback
    conn->WriteCallBack = CallBack;
    // if re enter only change callback
    if ( conn->writeType == MJCONN_WRITE ) return true;

    conn->writeType = MJCONN_WRITE;
    return mjConn_AddWriteEvent( conn );
}

/*
==========================================================
mjConn_WriteS
    write string
==========================================================
*/
bool mjConn_WriteS( mjConn conn, char* buf, mjProc CallBack )
{
    if ( !conn || !buf ) {
        MJLOG_ERR( "conn or buf is null" );
        return false;
    }
    mjConn_BufWriteS( conn, buf );
    return mjConn_Flush( conn, CallBack );
}

/*
==========================================================
mjConn_Write
    write mjStr
==========================================================
*/
bool mjConn_Write( mjConn conn, mjStr buf, mjProc CallBack )
{
    if ( !conn || !buf ) {
        MJLOG_ERR( "conn or buf is null" );
        return false;
    }
    return mjConn_WriteS( conn, buf->data, CallBack );
}

/*
===============================================================
mjConn_ThreadFin
    when thread finish, call this function
===============================================================
*/
static void* mjConn_ThreadFin( void* arg )
{
    mjConn conn = ( mjConn ) arg;
    
    // close pipe fd
    mjEV_Del( conn->ev, conn->threadReadNotify, MJEV_READABLE );
    close( conn->threadReadNotify );
    close( conn->threadWriteNotify );
    conn->threadReadNotify = -1;
    conn->threadWriteNotify = -1;

    conn->ThreadRoutine = NULL;
    // error happend in thread, exit
    if ( conn->closed || conn->error ) {
        MJLOG_ERR( "error happends in thread" );
        mjConn_Delete( conn );
        return NULL;
    }
    // reset thread type
    conn->threadType = MJCONN_NONE;
    // call thread callback
    if ( conn->ThreadCallBack ) conn->ThreadCallBack( conn );
    return NULL;
}

/*
==================================================
mjConn_Thread
    main thread, call ThreadRoutine
==================================================
*/
static void* mjConn_Thread( void* arg )
{
    mjConn conn = ( mjConn ) arg;
    // if error happend, set conn->error
    if ( conn->ThreadRoutine ) {
        conn->ThreadRoutine( conn );
    }
    // notify thread finish
    write( conn->threadWriteNotify, "OK", 2 ); 
    return NULL;
}

/*
=====================================================================
mjConn_RunAsync
    run async, we can't call any conn function in routine, only
    change the data
=====================================================================
*/
bool mjConn_RunAsync( mjConn conn, mjProc Routine, mjProc CallBack )
{
    // RunAsync can't re enter
    if ( conn->threadType != MJCONN_NONE ) {
        MJLOG_ERR( "threadType must be MJCONN_NONE" );
        goto failout;
    }
    // must have callback
    if ( !CallBack ) {
        MJLOG_ERR( "proc is null" );
        goto failout;
    }
    // create notify fd
    int notify[2];
    int ret = pipe( notify );
    if ( ret < 0 ) {
        MJLOG_ERR( "pipe error: %s", strerror( errno )  );
        goto failout;
    }
    conn->threadReadNotify  = notify[0];
    conn->threadWriteNotify = notify[1];
    // set thread status
    conn->threadType        = MJCONN_THREAD;
    conn->ThreadCallBack    = CallBack;
    conn->ThreadRoutine     = Routine;
    // add thread notify to eventloop
    mjEV_Add( conn->ev, conn->threadReadNotify, MJEV_READABLE, mjConn_ThreadFin, conn );
    // create and run thread
    mjThread_RunOnce( mjConn_Thread, conn );
    return true;

failout:
    MJLOG_ERR( "RunAsync Error" );
    mjConn_Delete( conn );
    return false;
}

/*
==================================================================
mjConn_SetConnectTimeout
    set conn connect timeout
==================================================================
*/
bool mjConn_SetConnectTimeout( mjConn conn, unsigned int connectTimeout )
{
    if ( !conn ) {
        MJLOG_ERR( "conn is null" );
        return false;
    }
    conn->connectTimeout = connectTimeout;
    return true;
}

/*
===========================================================================
mjConn_SetTimeout
    set conn, read and write timeout
===========================================================================
*/
bool mjConn_SetTimeout( mjConn conn, unsigned int readTimeout, unsigned int writeTimeout )
{
    if ( !conn ) {
        MJLOG_ERR( "conn is null" );
        return false;
    }
    conn->readTimeout   = readTimeout;
    conn->writeTimeout  = writeTimeout;
    return true;
}

/*
=========================================================================
mjConn_SetPrivate
    set conn private data and private free function
=========================================================================
*/
bool mjConn_SetPrivate( mjConn conn, void* private, mjProc FreePrivte )
{
    if ( !conn ) {
        MJLOG_ERR( "conn is null" );
        return false;
    }
    conn->private       = private;
    conn->FreePrivte    = FreePrivte;
    return true;
}

/*
=====================================================
mjConn_SetServer
    set conn server, when conn in server side
=====================================================
*/
bool mjConn_SetServer( mjConn conn, void* server )
{
    if ( !conn ) {
        MJLOG_ERR( "conn is null" );
        return false;
    }
    conn->server    = server;
    return true;
}

/*
==================================================================
mjConn_DelConnectEvent
    del connect event
==================================================================
*/
static void* mjConn_DelConnectEvent( mjConn conn )
{
    mjEV_Del( conn->ev, conn->fd, MJEV_READABLE | MJEV_WRITEABLE ); 
    if ( conn->connectTimeout ) {
        mjEV_DelTimer( conn->ev, conn->connectTimeoutEvent );
        conn->connectTimeout        = 0;
        conn->connectTimeoutEvent   = NULL;
    }
    conn->connectType = MJCONN_NONE;    
    if ( conn->ConnectCallback ) conn->ConnectCallback( conn );
    return NULL;
}

/*
===================================================================
mjConn_ConnectEventCallback
    connect callback
===================================================================
*/
static void* mjConn_ConnectEventCallback( void* arg )
{
    mjConn conn = ( mjConn )arg;
    int err = 0;
    socklen_t errlen = sizeof( err );
    // get socket status
    if ( getsockopt( conn->fd, SOL_SOCKET, SO_ERROR, &err, &errlen ) == -1 ) {
        MJLOG_ERR( "getsockopt error, %s", strerror( errno ) );
        mjConn_Delete( conn );
        return NULL;
    }
    if ( err ) {
        MJLOG_ERR( "err is: %s", strerror( err ) );
        mjConn_Delete( conn );
        return NULL;
    }
    // connect success
    mjConn_DelConnectEvent( conn ); 
    return NULL;
}

/*
=============================================================
mjConn_AddConnectEvent
    add connect event 
=============================================================
*/
static bool mjConn_AddConnectEvent( mjConn conn )
{
    // add to eventloop
    int ret = mjEV_Add( conn->ev, conn->fd, MJEV_READABLE | MJEV_WRITEABLE, 
                    mjConn_ConnectEventCallback, conn );
    if ( ret < 0 ) {
        MJLOG_ERR( "mjEV_Add error" );
        goto failout;
    }
    // set connect timeout
    if ( conn->connectTimeout ) {
        conn->connectTimeoutEvent = 
            mjEV_AddTimer(conn->ev, conn->connectTimeout, mjConn_TimeoutCallBack, conn);
        if ( !conn->connectTimeoutEvent ) {
            MJLOG_ERR("mjEV_AddTimer error");
            goto failout;
        }
    }
    return true;

failout:
    mjConn_Delete(conn);
    return false;
}

/*
============================================================
mjConn_Connect
    connect to host async
============================================================
*/
bool mjConn_Connect( mjConn conn, const char* ipaddr, int port, mjProc CallBack )
{
    // sanity check
    if ( !conn || !CallBack ) {
        MJLOG_ERR("conn or proc is null");
        return false;
    }
    // can't re enter
    if ( conn->connectType != MJCONN_NONE ) {
        MJLOG_ERR( "connectType must be MJCONN_NONE" );
        return false;
    }
    // set conn type and callback
    conn->connectType      = MJCONN_CONN;
    conn->ConnectCallback  = CallBack;
    // init address
    struct sockaddr_in addr;
    bzero( &addr, sizeof( addr ) );
    addr.sin_family = AF_INET;
    addr.sin_port   = htons( port );
    inet_pton( AF_INET, ipaddr, &addr.sin_addr );
    // try to connect 
    int ret = connect( conn->fd, ( struct sockaddr* )&addr, sizeof( addr ) );
    if ( ret == 0 ) {             // connect success
        conn->connectType = MJCONN_NONE;
        if ( conn->ConnectCallback ) conn->ConnectCallback( conn );
        return true;
    }
    // connect failed, set nonblock connect
    if ( errno == EINPROGRESS ) return mjConn_AddConnectEvent( conn );
    MJLOG_ERR( "connect failed" );  
    mjConn_Delete( conn );
    return false;
}

/*
=================================================
mjConn_New
    create mjConn
    return NULL -- fail, other -- success
================================================
*/
mjConn mjConn_New( mjev ev, int fd )
{
    // event loop must not be null
    if ( !ev ) {
        MJLOG_ERR( "ev is null" );
        return NULL;   
    }
    // set fd to nonblock 
    mjSock_SetBlocking( fd, 0 );
    // alloc mjConn struct 
    mjConn conn = ( mjConn ) calloc( 1, sizeof( struct mjConn ) );
    if ( !conn ) {
        MJLOG_ERR( "alloc mjConn error" );
        return NULL;
    }

    conn->fd        = fd;           // set conn fd 
    conn->server    = NULL;         // set server
    conn->ev        = ev;           // set ev
    // init timeout event
    conn->connectTimeout        = 0;   
    conn->readTimeout           = 0;    
    conn->writeTimeout          = 0;
    conn->connectTimeoutEvent   = NULL; 
    conn->readTimeoutEvent      = NULL;
    conn->writeTimeoutEvent     = NULL;
    // create buffer
    conn->rbuf = mjStr_New();
    conn->wbuf = mjStr_New();
    conn->data = mjStr_New();
    if ( !conn->rbuf || !conn->wbuf || !conn->data ) {
        MJLOG_ERR( "alloc mjStr error" );
        mjStr_Delete( conn->rbuf );
        mjStr_Delete( conn->wbuf );
        mjStr_Delete( conn->data );
        free( conn );
        return NULL;
    }
    // set connect status
    conn->connectType       = MJCONN_NONE;
    conn->ConnectCallback   = NULL;
    // set read status
    conn->readType          = MJCONN_NONE;   
    conn->ReadCallBack      = NULL;
    conn->delim             = NULL;
    conn->rbytes            = -1;
    // set write status
    conn->writeType         = MJCONN_NONE;
    conn->WriteCallBack     = NULL;
    // set thread status
    conn->threadType        = MJCONN_NONE;
    conn->ThreadCallBack    = NULL;
    conn->threadReadNotify  = -1;
    conn->threadWriteNotify = -1;           // thread notify fd 
    conn->ThreadRoutine     = NULL;         // thread routine

    conn->error             = 0;            // no error 
    conn->closed            = 0;            // read fd is not closed 

    conn->FreePrivte        = NULL;
    conn->private           = NULL;

    return conn;
}

/*
============================================
mjConn_Delete
    delete mjConn struct
============================================
*/
void mjConn_Delete( mjConn conn )
{
    if ( !conn ) {
        MJLOG_ERR( "conn is null" );
        return;
    }
    // invalid connect timeout event
    if ( conn->connectTimeout ) {                            
        mjEV_DelTimer( conn->ev, conn->connectTimeoutEvent );
        conn->connectTimeout    = 0;
        conn->readTimeoutEvent  = NULL;
    }
    // invalid read timeout event
    if ( conn->readTimeout ) {
        mjEV_DelTimer( conn->ev, conn->readTimeoutEvent ); 
        conn->readTimeout       = 0;
        conn->readTimeoutEvent  = NULL;
    }
    // invalid write timeout event
    if ( conn->writeTimeout ) {
        mjEV_DelTimer( conn->ev, conn->writeTimeoutEvent );
        conn->writeTimeout      = 0;
        conn->writeTimeoutEvent = NULL;
    }
    // free private data
    if ( conn->private && conn->FreePrivte ) { 
        conn->FreePrivte( conn->private );
    }
    // close threadNotify
    if ( conn->threadReadNotify != -1 ) close( conn->threadReadNotify );
    if ( conn->threadWriteNotify != -1 ) close( conn->threadWriteNotify );
    // delete eventloop fd
    mjEV_Del( conn->ev, conn->fd, MJEV_READABLE | MJEV_WRITEABLE );
    mjSock_Close( conn->fd );

    mjStr_Delete( conn->rbuf );
    mjStr_Delete( conn->wbuf );
    mjStr_Delete( conn->data );
    free( conn );
}

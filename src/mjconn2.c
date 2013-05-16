#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "mjconn2.h"
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
mjConn2_TimeoutCallBack
    read/write timeout callback
=================================================
*/
static void* mjConn2_TimeoutCallBack( void* data ) {
    mjConn2 conn = ( mjConn2 ) data;
    MJLOG_ERR( "timeout" );
    mjConn2_Delete( conn );
    return NULL;
}

/*
====================================================
mjConn2_DelReadEvent
    read finish del event and run callback
====================================================
*/
static void* mjConn2_DelReadEvent( mjConn2 conn )
{
    // del read event
    mjEV2_Del( conn->ev, conn->fd, MJEV_READABLE );
    if ( conn->readTimeout ) {
        mjEV2_DelTimer( conn->ev, conn->readTimeoutEvent ); // invalid timer event
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
mjConn2_ReadEventCallBack
    run when data come
    when error or closed, close conn
=======================================================
*/
static void* mjConn2_ReadEventCallBack( void* arg )
{
    mjConn2 conn = ( mjConn2 )arg;
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
            mjConn2_DelReadEvent( conn );
            return NULL;
        }
    } else if ( conn->readType == MJCONN_READUNTIL ) { // read type is readuntil
        int pos = mjStr_Search( conn->rbuf, conn->delim );
        if ( pos != -1 ) {
            mjStr_CopyB( conn->data, conn->rbuf->data, pos );
            mjStr_Consume( conn->rbuf, pos + strlen( conn->delim ) );
            mjConn2_DelReadEvent( conn );
            return NULL;
        }
    } else if ( conn->readType == MJCONN_READ ) { // read type is normal read
        if ( conn->rbuf && conn->rbuf->length > 0 ) {
            mjStr_CopyB( conn->data, conn->rbuf->data, conn->rbuf->length );
            mjStr_Consume( conn->rbuf, conn->rbuf->length );
            mjConn2_DelReadEvent( conn );
            return NULL;
        }
    }
    // some error happend, close conn
    if ( conn->closed || conn->error ) {
        mjConn2_Delete( conn );
    }
    return NULL;
}

/*
==============================================
mjConn2_AddReadEvent
    add read event, read timeout event
    return false -- add error, close conn
            true -- add success
==============================================
*/
static bool mjConn2_AddReadEvent( mjConn2 conn ) {
    // check if conn has closed or error
    if ( conn->closed || conn->error ) {
        MJLOG_ERR( "conn is closed or error" );
        goto failout;
    }
    // add readevent
    if ( mjEV2_Add( conn->ev, conn->fd, MJEV_READABLE, 
            mjConn2_ReadEventCallBack, conn ) < 0 ) {
        MJLOG_ERR( "mjEV2_Add error" );
        goto failout;
    }
    // add read timeout event
    if ( conn->readTimeout ) {
        conn->readTimeoutEvent = mjEV2_AddTimer( conn->ev, 
            conn->readTimeout, mjConn2_TimeoutCallBack, conn );
        if ( !conn->readTimeoutEvent ) {
            MJLOG_ERR( "mjEV2_AddTimer error" );
            goto failout;
        }
    }
    return true;

failout:
    mjConn2_Delete( conn );
    return false;
}

/*
=========================================================
mjConn2_ReadBytes
    read len bytes
    return false -- error, true -- success
=========================================================
*/
bool mjConn2_ReadBytes( mjConn2 conn, int len, mjProc CallBack ) {
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
        if ( conn->ReadCallBack ) {
            mjEV2_AddPending( conn->ev, conn->ReadCallBack( conn ), conn );
        }
        return true;
    }
    // add to event loop
    return mjConn2_AddReadEvent( conn );
}

/*
==============================================================
mjConn2_ReadUntil
    mjConn2 read until delim 
    return false --- error, true -- readfinish or set event ok
==============================================================
*/
bool mjConn2_ReadUntil( mjConn2 conn, char* delim, mjProc CallBack )
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
    return mjConn2_AddReadEvent( conn ); 
}

/*
===========================================================
mjConn2_Read
    read data
===========================================================
*/
bool mjConn2_Read( mjConn2 conn, mjProc CallBack )
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
    return mjConn2_AddReadEvent( conn );
}

/*
==========================================================
mjConn2_DelWriteEvent
    del write event
==========================================================
*/
static void* mjConn2_DelWriteEvent( mjConn2 conn )
{
    mjEV2_Del( conn->ev, conn->fd, MJEV_WRITEABLE );
    // del write timeout event
    if ( conn->writeTimeout ) {
        mjEV2_DelTimer( conn->ev, conn->writeTimeoutEvent );
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
mjConn2_WriteEventCallback
    run when we can write data
===========================================================
*/
static void* mjConn2_WriteEventCallback( void* arg)
{
    mjConn2 conn = ( mjConn2 )arg;
    int ret = write( conn->fd, conn->wbuf->data, conn->wbuf->length );
    if ( ret < 0 ) {
        MJLOG_ERR( "conn write error: %s", strerror( errno ) );
        mjConn2_Delete( conn );
        return NULL;
    }
    mjStr_Consume( conn->wbuf, ret );
    // no data to write call DelWriteEvent
    if ( conn->wbuf->length == 0 ) {
        mjConn2_DelWriteEvent( conn );
    }
    return NULL;
}

/*
==================================================
mjConn2_AddWriteEvent
    add write event to eventloop
==================================================
*/
static bool mjConn2_AddWriteEvent( mjConn2 conn )
{
    // sanity check
    if ( conn->closed || conn->error ) {
        MJLOG_ERR( "conn is closed or error" );
        goto failout;
    }
    // add write event
    if ( mjEV2_Add( conn->ev, conn->fd, MJEV_WRITEABLE, 
            mjConn2_WriteEventCallback, conn ) < 0 ) {
        MJLOG_ERR( "mjEV2_Add error" );
        goto failout;
    }
    // AddWriteEvent can be call many times
    // When we call it twice, we can't change the callback
    if ( conn->writeTimeout && !conn->writeTimeoutEvent ) { 
        conn->writeTimeoutEvent = mjEV2_AddTimer( conn->ev, 
                conn->writeTimeout, mjConn2_TimeoutCallBack, conn );
        if ( !conn->writeTimeoutEvent ) {
            MJLOG_ERR( "mjEV2_AddTimer error" );
            goto failout;
        }
    }
    return true;

failout:
    mjConn2_Delete( conn );
    return false;
}

/*
===============================================
mjConn2_BufWriteS
    copy string to wbuf
===============================================
*/
bool mjConn2_BufWriteS( mjConn2 conn, char* buf) {
    if ( !conn || !buf ) {
        MJLOG_ERR( "conn or buf is null" );
        return false;
    }
    mjStr_CatS( conn->wbuf, buf );
    return true;
}

/*
==============================================
mjConn2_BufWrite
    copy mjStr to wbuf
==============================================
*/
bool mjConn2_BufWrite( mjConn2 conn, mjStr buf ) {
    if ( !conn || !buf ) {
        MJLOG_ERR( "conn or buf is null" );
        return false;
    }
    return mjConn2_BufWriteS( conn, buf->data );
}

/*
===============================================
mjConn2_Flush
    flush wbuf
===============================================
*/
bool mjConn2_Flush( mjConn2 conn, mjProc CallBack ) {
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
    return mjConn2_AddWriteEvent( conn );
}

/*
==========================================================
mjConn2_WriteS
    write string
==========================================================
*/
bool mjConn2_WriteS( mjConn2 conn, char* buf, mjProc CallBack ) {
    if ( !conn || !buf ) {
        MJLOG_ERR( "conn or buf is null" );
        return false;
    }
    mjConn2_BufWriteS( conn, buf );
    return mjConn2_Flush( conn, CallBack );
}

/*
==========================================================
mjConn2_Write
    write mjStr
==========================================================
*/
bool mjConn2_Write( mjConn2 conn, mjStr buf, mjProc CallBack ) {
    if ( !conn || !buf ) {
        MJLOG_ERR( "conn or buf is null" );
        return false;
    }
    return mjConn2_WriteS( conn, buf->data, CallBack );
}

/*
=============================================
mjConn2_SetConnectTimeout
    set conn connect timeout
=============================================
*/
bool mjConn2_SetConnectTimeout( mjConn2 conn, 
            unsigned int connectTimeout ) {
    if ( !conn ) {
        MJLOG_ERR( "conn is null" );
        return false;
    }
    conn->connectTimeout = connectTimeout;
    return true;
}

/*
=================================================================
mjConn2_SetTimeout
    set conn, read and write timeout
=================================================================
*/
bool mjConn2_SetTimeout( mjConn2 conn, unsigned int readTimeout, 
            unsigned int writeTimeout ) {
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
mjConn2_SetPrivate
    set conn private data and private free function
=========================================================================
*/
bool mjConn2_SetPrivate( mjConn2 conn, void* private, mjProc FreePrivte ) {
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
mjConn2_SetServer
    set conn server, when conn in server side
=====================================================
*/
bool mjConn2_SetServer( mjConn2 conn, void* server ) {
    if ( !conn ) {
        MJLOG_ERR( "conn is null" );
        return false;
    }
    conn->server    = server;
    return true;
}

/*
==================================================================
mjConn2_DelConnectEvent
    del connect event
==================================================================
*/
static void* mjConn2_DelConnectEvent( mjConn2 conn ) {
    mjEV2_Del( conn->ev, conn->fd, MJEV_READABLE | MJEV_WRITEABLE ); 
    if ( conn->connectTimeout ) {
        mjEV2_DelTimer( conn->ev, conn->connectTimeoutEvent );
        conn->connectTimeout        = 0;
        conn->connectTimeoutEvent   = NULL;
    }
    conn->connectType = MJCONN_NONE;    
    if ( conn->ConnectCallback ) conn->ConnectCallback( conn );
    return NULL;
}

/*
===================================================================
mjConn2_ConnectEventCallback
    connect callback, successful
===================================================================
*/
static void* mjConn2_ConnectEventCallback( void* arg ) {
    mjConn2 conn = ( mjConn2 )arg;
    int err = 0;
    socklen_t errlen = sizeof( err );
    // get socket status
    if ( getsockopt( conn->fd, SOL_SOCKET, SO_ERROR, &err, &errlen ) == -1 ) {
        MJLOG_ERR( "getsockopt error, %s", strerror( errno ) );
        mjConn2_Delete( conn );
        return NULL;
    }
    if ( err ) {
        MJLOG_ERR( "err is: %s", strerror( err ) );
        mjConn2_Delete( conn );
        return NULL;
    }
    // connect success
    mjConn2_DelConnectEvent( conn ); 
    return NULL;
}

/*
=============================================================
mjConn2_AddConnectEvent
    add connect event 
=============================================================
*/
static bool mjConn2_AddConnectEvent( mjConn2 conn ) {
    // add to eventloop
    if ( mjEV2_Add( conn->ev, conn->fd, MJEV_READABLE | MJEV_WRITEABLE, 
                mjConn2_ConnectEventCallback, conn ) < 0 )  {
        MJLOG_ERR( "mjEV2_Add error" );
        goto failout;
    }
    // set connect timeout
    if ( conn->connectTimeout ) {
        conn->connectTimeoutEvent = mjEV2_AddTimer( conn->ev, 
                conn->connectTimeout, mjConn2_TimeoutCallBack, conn );
        if ( !conn->connectTimeoutEvent ) {
            MJLOG_ERR("mjEV2_AddTimer error");
            goto failout;
        }
    }
    return true;

failout:
    mjConn2_Delete(conn);
    return false;
}

/*
============================================================
mjConn2_Connect
    connect to host async
============================================================
*/
bool mjConn2_Connect( mjConn2 conn, const char* ipaddr, 
            int port, mjProc CallBack ) {
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
    if ( ret == 0 ) {             
        // connect success
        conn->connectType = MJCONN_NONE;
        if ( conn->ConnectCallback ) {
            mjEV2_AddPending( conn->ev, conn->ConnectCallback, conn );
        }
        return true;
    }
    // connect failed, set nonblock connect
    if ( errno == EINPROGRESS ) return mjConn2_AddConnectEvent( conn );
    MJLOG_ERR( "connect failed" );  
    mjConn2_Delete( conn );
    return false;
}

// conn buffer
#define MAX_FD      60000
static struct mjConn2 _conn[MAX_FD];

/*
==================================================
mjConn2_SetBuffer
    used by mjConn2_New for init buffer
==================================================
*/
static mjStr mjConn2_SetBuffer( mjStr defVal ) {
    if ( defVal ) return defVal;
    return mjStr_New();
}

/*
=================================================
mjConn2_New
    create mjConn2
    return NULL -- fail, other -- success
================================================
*/
mjConn2 mjConn2_New( mjEV2 ev, int fd ) {
    // event loop must not be null
    if ( !ev ) {
        MJLOG_ERR( "ev is null" );
        return NULL;   
    }
    if ( fd > MAX_FD ) {
        MJLOG_ERR( "fd is too large" );
        return NULL;
    }
    // set fd to nonblock 
    mjSock_SetBlocking( fd, 0 );
    // alloc mjConn2 struct 
    mjConn2 conn = &_conn[fd];
    mjStr rbak = conn->rbuf;
    mjStr wbak = conn->wbuf;
    mjStr dbak = conn->data;
    // clean mjconn
    memset( conn, 0, sizeof( struct mjConn2 ) );
    conn->fd = fd;           // set conn fd 
    conn->ev = ev;           // set ev
    // create buffer
    conn->rbuf = mjConn2_SetBuffer( rbak );
    conn->wbuf = mjConn2_SetBuffer( wbak );
    conn->data = mjConn2_SetBuffer( dbak );
    if ( !conn->rbuf || !conn->wbuf || !conn->data ) {
        MJLOG_ERR( "mjStr create error" );
        mjSock_Close( fd );
        return NULL;
    }
    return conn;
}

/*
============================================
mjConn2_Delete
    delete mjConn2 struct
============================================
*/
bool mjConn2_Delete( mjConn2 conn ) {
    if ( !conn ) {
        MJLOG_ERR( "conn is null" );
        return false;
    }
    // invalid connect timeout event
    if ( conn->connectTimeout ) {                            
        mjEV2_DelTimer( conn->ev, conn->connectTimeoutEvent );
        conn->connectTimeout        = 0;
        conn->connectTimeoutEvent   = NULL;
    }
    // invalid read timeout event
    if ( conn->readTimeout ) {
        mjEV2_DelTimer( conn->ev, conn->readTimeoutEvent ); 
        conn->readTimeout       = 0;
        conn->readTimeoutEvent  = NULL;
    }
    // invalid write timeout event
    if ( conn->writeTimeout ) {
        mjEV2_DelTimer( conn->ev, conn->writeTimeoutEvent );
        conn->writeTimeout      = 0;
        conn->writeTimeoutEvent = NULL;
    }
    // free private data
    if ( conn->private && conn->FreePrivte ) { 
        conn->FreePrivte( conn->private );
    }
    // delete eventloop fd, pending proc
    mjEV2_Del( conn->ev, conn->fd, MJEV_READABLE | MJEV_WRITEABLE );
    mjEV2_DelPending( conn->ev, conn );
    mjSock_Close( conn->fd );
    return true;
}

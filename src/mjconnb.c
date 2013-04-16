#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>

#include "mjconnb.h"
#include "mjlog.h"
#include "mjsock.h"

#define MJCONNB_NONE        0
#define MJCONNB_READ        1
#define MJCONNB_READBYTES   2 
#define MJCONNB_READUNTIL   3
#define BUF_SIZE            4096


/*
==========================================================
mjConnB_ReadToBuf
    read data to buffer
    return  -3 --- too bad, should not be this
            -2 --- readtimeout, get some data
            -1 --- readerror, get some data
             0 --- peer close, get some data
            other --- get data
==========================================================
*/
static int mjConnB_ReadToBuf( mjConnB conn, mjStr data )
{
    int ret = -3;
    char buf[BUF_SIZE];
	// read data first in a loop
    for ( ;; ) {
        // buffer has enough data, copy and return
        if ( conn->readtype == MJCONNB_READBYTES ) {
            if ( conn->rbytes <= conn->rbuf->length ) { 
                mjStr_CopyB(data, conn->rbuf->data, conn->rbytes);
                mjStr_Consume(conn->rbuf, conn->rbytes);
                return data->length;
            }
        } else if ( conn->readtype == MJCONNB_READUNTIL ) {
            int pos = mjStr_Search( conn->rbuf, conn->delim );
            if ( pos != -1 ) {
                mjStr_CopyB( data, conn->rbuf->data, pos );
                mjStr_Consume( conn->rbuf, pos + strlen( conn->delim ) );
                return data->length;
            }
        } else if ( conn->readtype == MJCONNB_READ ) {
            if ( conn->rbuf && conn->rbuf->length > 0 ) {
                mjStr_CopyB( data, conn->rbuf->data, conn->rbuf->length );
                mjStr_Consume( conn->rbuf, conn->rbuf->length );
                return data->length;
            }
        }
        // we must read data  
        ret = read( conn->fd, buf, BUF_SIZE );
        if ( ret == -1 ) {                   
            if ( errno == EINTR ) continue;                     // intrupt by signal try again
            if ( errno == EAGAIN || errno == EWOULDBLOCK ) {    // read timeout, set ret and break
                MJLOG_ERR( "read timeout" );
                ret = -2;
            }
            break;              // other error, break
        }
        if ( ret == 0 )  break;     //read close, break, copy data to rbuf
        // read ok put data to rbuf, try again
        mjStr_CatB( conn->rbuf, buf, ret );
    }
    // read error or read close, copy data
    mjStr_Copy( data, conn->rbuf );
    mjStr_Consume( conn->rbuf, conn->rbuf->length );

    return ret;
}

int mjConnB_Read( mjConnB conn, mjStr data )
{
    // sanity check
    if ( !conn || !data ) {
        MJLOG_ERR( "sanity check error" );
        return -1;
    }
    conn->readtype  =   MJCONNB_READ;
    return mjConnB_ReadToBuf( conn, data );
}

/* 
===========================================================
mjConnB_ReadBytes
    read len size bytes 
===========================================================
*/
int mjConnB_ReadBytes( mjConnB conn, mjStr data, int len )
{
    if ( !conn || !data || len <= 0 ) {
        MJLOG_ERR( "sanity check error" );
        return -1;
    }
    conn->readtype  = MJCONNB_READBYTES;        
    conn->rbytes    = len;
    return mjConnB_ReadToBuf( conn, data );
}

/* 
====================================================================
mjConnB_ReadUntil
    read data until delim 
====================================================================
*/
int mjConnB_ReadUntil( mjConnB conn, const char* delim, mjStr data )
{
    if ( !conn || !data || !delim ) {
        MJLOG_ERR( "sanity check error" );
        return -1;
    }
    conn->readtype  = MJCONNB_READUNTIL;
    conn->delim     = delim;
    return mjConnB_ReadToBuf( conn, data );
}

/*
==========================================================
mjConnB_Write
    write data to conn
==========================================================
*/
int mjConnB_Write( mjConnB conn, mjStr data )
{
    if ( !conn || !data || !data->length ) {
        MJLOG_ERR( "sanity check error" );
        return -1;
    }
    return mjConnB_WriteB( conn, data->data, data->length );
}

int mjConnB_WriteB( mjConnB conn, char* buf , int length )
{
    if ( !conn || !buf || !length ) {
        MJLOG_ERR( "sanity check error");
        return -1;
    }
    return write( conn->fd, buf, length );
}

int mjConnB_WriteS( mjConnB conn, char* buf )
{
    if ( !conn || !buf ) {
        MJLOG_ERR( "sanity check error");
        return -1;
    }
    return mjConnB_WriteB( conn, buf, strlen( buf ) );
}

/*
============================================================
mjConnB_SetServer
    set conn server 
============================================================
*/
bool mjConnB_SetServer( mjConnB conn, void* server)
{
    if ( !conn ) {
        MJLOG_ERR( "conn is null" );
        return false;
    }
    conn->server = server;
    return true;
}

/*
=============================================================
mjConnB_SetTimeout
    set mjConnB read and write timeout
    return      -1 -- set error
                0  -- success
=============================================================
*/
bool mjConnB_SetTimeout( mjConnB conn, unsigned int readTimeout, 
            unsigned int writeTimeout )
{
    if ( !conn ) {
        MJLOG_ERR( "conn is null" );
        return false;
    }
    //set read timeout    
    if ( readTimeout ) {
        struct timeval tv;
        tv.tv_sec = readTimeout / 1000;
        tv.tv_usec = ( readTimeout % 1000 ) * 1000;
        // set recv timeout
        if ( setsockopt( conn->fd, SOL_SOCKET, SO_RCVTIMEO, 
                &tv, sizeof( tv ) ) < 0 ) {
            MJLOG_ERR( "setsockopt error" );
            return false;
        }
    }
    if ( writeTimeout ) {
        struct timeval tv;
        tv.tv_sec = writeTimeout / 1000;
        tv.tv_usec = ( writeTimeout % 1000 ) * 1000;
        // set send timeout
        if ( setsockopt( conn->fd, SOL_SOCKET, SO_SNDTIMEO, 
                &tv, sizeof( tv ) ) < 0 ) {
            MJLOG_ERR( "setsockopt error" );
            return false;
        }
    }

    return true;
}

/*
=================================================
mjConnB_SetPrivate
    set conn private data and FreePrivate fun
=================================================
*/
bool mjConnB_SetPrivate( mjConnB conn, void* private, mjProc FreePrivate )
{
    if ( !conn ) {
        MJLOG_ERR( "conn is null" );
        return false;
    }
    conn->private       = private;
    conn->FreePrivate   = FreePrivate;
    return true;
}

// conn buffer
#define MAX_FD              60000
static struct mjConnB _conn[MAX_FD];

/*
=========================================================
mjConnB_New
    input socket fd, output mjConnB struct
    return NULL -- fail, other -- success
=========================================================
*/
mjConnB mjConnB_New( int fd )
{
    // sanity check
    if ( fd >= MAX_FD ) {
        MJLOG_ERR( "fd is too large" );
        return NULL;
    }
    // set fd to block
    mjSock_SetBlocking( fd, 1 );
    // get mjConnB struct
    mjConnB conn    = &_conn[fd];
    conn->fd        = fd;            
    conn->server    = NULL;
    // create rbuf
    if ( !conn->rbuf ) {
        // create read buffer
        conn->rbuf = mjStr_New();
        if ( !conn->rbuf ) {
            MJLOG_ERR( "mjStr create error" );
            mjSock_Close( fd );
            return NULL;
        }
    }
    conn->rbuf->length  = 0;
    // init read
    conn->readtype      = MJCONNB_NONE;
    conn->delim         = NULL;
    conn->rbytes        = -1;
    // init private
    conn->FreePrivate   = NULL;
    conn->private       = NULL;

    return conn;
}

/*
===============================================================
mjConnB_ConnectReady
    Used by mjConnB_Connect for connect timeout
    return  0 -- connect timeout or poll failed
            1 -- connect ok
===============================================================
*/
static int mjConnB_ConnectReady( int fd, unsigned int timeout )
{
    struct pollfd wfd[1];
    wfd[0].fd       = fd; 
    wfd[0].events   = POLLOUT;

    long msc = -1;
    if ( timeout ) msc = timeout;

    int ret = poll( wfd, 1, msc );
    if ( ret == -1 ) {
        MJLOG_ERR( "poll error" );
        return 0;
    }
    if ( ret == 0 ) {
        MJLOG_ERR( "poll timeout" );
        return 0;
    } 

    return 1;
}


/*
==============================================================================
mjConnB_Connect
    conn to addr and port
    return NULL -- fail, other -- success
==============================================================================
*/
mjConnB mjConnB_Connect( const char* addr, int port, unsigned int timeout )
{
    char _port[6];
    snprintf( _port, 6, "%d", port );

    struct addrinfo hints;
    memset( &hints, 0, sizeof( hints ) );
    hints.ai_family     = AF_INET;
    hints.ai_socktype   = SOCK_STREAM;

    struct addrinfo* servinfo;
    int rv = getaddrinfo( addr, _port, &hints, &servinfo );
    if ( rv != 0 ) {
        MJLOG_ERR( "getaddrinfo called error" );
        return NULL;
    }


    struct addrinfo* p;
    for ( p = servinfo; p != NULL; p = p->ai_next ) {
        int fd = socket( p->ai_family, p->ai_socktype, p->ai_protocol );
        if ( fd == -1 ) {
            MJLOG_ERR( "socket error" );
            continue;
        }
        // set to nonblock
        mjSock_SetBlocking( fd, 0 );
        if ( connect( fd, p->ai_addr, p->ai_addrlen ) == -1 ) {
            if ( errno == EHOSTUNREACH ) {
                close( fd );
                continue;
            } else if ( errno != EINPROGRESS ) {
                close( fd );
                continue;
            }
            // errno == EINPROGRESS
            if ( !mjConnB_ConnectReady( fd, timeout ) ) {
                MJLOG_ERR( "conn timeout" );
                close( fd );
                continue;
            }
        } 
        // connect ok return mjConnB
        return mjConnB_New( fd );
    }

    return NULL;
}

/*
==============================================
mjConnB_Delete
    delete conn struct
    no return
==============================================
*/
void mjConnB_Delete( mjConnB conn )
{
    // sanity check
    if ( !conn ) {
        MJLOG_ERR( "conn is null" );
        return;
    }
    // free private data
    if ( conn->private && conn->FreePrivate ) {
        conn->FreePrivate( conn->private );
    }
    mjSock_Close( conn->fd );
}

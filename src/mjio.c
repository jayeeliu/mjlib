#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include "mjlog.h"
#include "mjio.h"

#define BUF_LEN 1024

/*
===============================================================================
mjIO_Read
    return mj file io
===============================================================================
*/
int mjIO_Read( mjIO io, mjStr data, int len ) {
    // sanity check
    if ( !io || !data || len <= 0 ) return 0;
    // buffer has data return
    if ( io->buffer && io->buffer->length > 0 ) {
        mjStr_Copy( data, io->buffer );
        mjStr_Consume( io->buffer, io->buffer->length );
        return data->length;
    }
    // io buffer is empty
    if ( len > BUF_LEN ) len = BUF_LEN;
    char buf[BUF_LEN];
    int ret = read( io->fd, buf, len );
    if ( ret < 0 ) {
        ret = -1;
        MJLOG_ERR( "data read error" );
    } else if ( ret == 0 ) {
        ret = 2;
    } else {
        mjStr_CopyB( data, buf, ret );
    }
    return ret;
}

/*
===============================================================================
mjIO_ReadLine
    read data from io
    return -1 -- read file error, has data
           -2 -- file close, has data 
           -3 -- can't be this
===============================================================================
*/
int mjIO_ReadLine( mjIO io, mjStr data ) {  
    int ret = -3;
    char buf[BUF_LEN];

    while ( 1 ) {
        int pos = mjStr_Search( io->buffer, "\n" );
        if ( pos != -1 ) {
            mjStr_CopyB( data, io->buffer->data, pos + 1 );
            mjStr_Consume( io->buffer, pos + 1 );
            return data->length;
        }
        // get data from file
        ret = read( io->fd, buf, BUF_LEN );
        if ( ret < 0 ) {
            ret = -1;
            MJLOG_ERR( "data read error" );
            break;
        }
        if ( !ret ) {
            ret = -2;
            break;
        }
        mjStr_CatB( io->buffer, buf, ret );
    }
    mjStr_Copy( data, io->buffer );
    mjStr_Consume( io->buffer, io->buffer->length );
    return ret;
}

/*
===============================================================================
mjIO_New
    creat mjIO struct
===============================================================================
*/
mjIO mjIO_New( const char* fileName) {
    // alloc mjio struct
    mjIO io = ( mjIO )calloc( 1, sizeof( struct mjIO ) );
    if ( !io ) {
        MJLOG_ERR( "mjio alloc error" );
        goto failout1;
    }
    // get fileName open file
    io->fileName = fileName;
    io->fd = open( io->fileName, O_RDWR );
    if ( io->fd < 0 ) {
        MJLOG_ERR( "open file error" );
        goto failout2;
    }
    // alloc mjio buffer
    io->buffer = mjStr_New();
    if ( !io->buffer ) {
        MJLOG_ERR( "mjStr_New error" );
        goto failout3;
    }
    return io;

failout3:
    close( io->fd );
failout2:
    free( io );
failout1:
    return NULL;
}

/*
===============================================================================
mjIO_Delete
    delete mjio struct
===============================================================================
*/
bool mjIO_Delete( mjIO io ) {
    if ( !io ) {
        MJLOG_ERR( "io is null" );
        return false;
    }
    close( io->fd );
    mjStr_Delete( io->buffer );
    free( io );
    return true;
}

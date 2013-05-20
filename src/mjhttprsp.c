#include <stdlib.h>

#include "mjlog.h"
#include "mjhttprsp.h"

/*
===============================================================================
mjHttpRsp_AddHeader
    add header to http response
===============================================================================
*/
bool mjHttpRsp_AddHeader( mjHttpRsp rsp, char* name, char* value ) {
    // sanity check
    if ( !rsp ) return false;
    // add value to header
    mjMap_AddS( rsp->rspHeader, name, value );
    return true;
}

/*
===============================================================================
mjHttpRsp_HeaderToStr
    change http header to string, alloc and return mjStr
===============================================================================
*/
mjStr mjHttpRsp_HeaderToStr( mjHttpRsp rsp ) {
    // alloc new mjStr
    mjStr str = mjStr_New();
    if ( !str ) {
        MJLOG_ERR( "mjStr_New error" );
        return NULL;
    }
    // iter the mjmap
    mjItem item = mjMap_GetNext( rsp->rspHeader, NULL );
    while ( item ) {
        mjStr_Cat( str, item->key );
        mjStr_CatS( str, ": " );
        mjStr_Cat( str, item->value ); 
        mjStr_CatS( str, "\r\n" );
        item = mjMap_GetNext( rsp->rspHeader, item );
    }
    return str;
}

/*
===============================================================================
mjHttpRsp_New
    create new mjHttpRsp struct
===============================================================================
*/
mjHttpRsp mjHttpRsp_New() {
    // alloc mjHttpRsp struct
    mjHttpRsp rsp = ( mjHttpRsp ) calloc( 1, sizeof( struct mjHttpRsp ) );
    if ( !rsp ) {
        MJLOG_ERR( "calloc error" );
        return NULL;
    }
    // alloc new mjMap
    rsp->rspHeader = mjMap_New( 128 );
    if ( !rsp->rspHeader ) {
        MJLOG_ERR( "mjMap_New error" );
        free( rsp );
        return NULL;
    }
    return rsp;
}

/*
===============================================================================
mjHttpRsp_Delete
    delete mjHttpRsp struct
===============================================================================
*/
bool mjHttpRsp_Delete( mjHttpRsp rsp ) {
    // sanity check
    if ( !rsp ) return false;
    // relase rspHeader
    if ( rsp->rspHeader ) mjMap_Delete( rsp ->rspHeader );
    free( rsp );
    return true;
}

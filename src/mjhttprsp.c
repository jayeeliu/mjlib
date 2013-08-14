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
    mjmap_adds( rsp->rspHeader, name, value );
    return true;
}

/*
===============================================================================
mjHttpRsp_HeaderToStr
    change http header to string, alloc and return mjstr
===============================================================================
*/
mjstr mjHttpRsp_HeaderToStr( mjHttpRsp rsp ) {
    // alloc new mjstr
    mjstr str = mjstr_new();
    if ( !str ) {
        MJLOG_ERR( "mjstr_New error" );
        return NULL;
    }
    // iter the mjmap
    mjitem item = mjmap_get_next( rsp->rspHeader, NULL );
    while ( item ) {
        mjstr_cat( str, item->key );
        mjstr_cats( str, ": " );
        mjstr_cat( str, item->value_str ); 
        mjstr_cats( str, "\r\n" );
        item = mjmap_get_next( rsp->rspHeader, item );
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
    rsp->rspHeader = mjmap_new( 128 );
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
    if ( rsp->rspHeader ) mjmap_delete( rsp ->rspHeader );
    free( rsp );
    return true;
}

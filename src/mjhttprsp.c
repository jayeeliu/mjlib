#include <stdlib.h>

#include "mjlog.h"
#include "mjhttprsp.h"

/*
=================================================================
mjHttpRsp_AddHeader
    add header to http response
=================================================================
*/
bool mjHttpRsp_AddHeader( mjHttpRsp rsp, char* name, char* value )
{
    if ( !rsp ) return false;

    mjstr tmp = mjstr_new();
    if ( !tmp ) {
        MJLOG_ERR( "mjstr_new error" );
        return false;
    }

    mjstr_copys( tmp, value );
    mjmap_add( rsp->rspheader, name, tmp );
    mjstr_delete( tmp );
    return true;
}

mjstr mjHttpRsp_HeaderToStr( mjHttpRsp rsp )
{
    mjstr str = mjstr_new();
    if ( !str ) {
        MJLOG_ERR( "mjstr_new error" );
        return NULL;
    }
    mjitem item = mjmap_GetNext( rsp->rspheader, NULL );
    while ( item ) {
        mjstr_cat( str, item->key );
        mjstr_cats( str, ": " );
        mjstr_cat( str, item->value ); 
        mjstr_cats( str, "\r\n" );
    
        item = mjmap_GetNext( rsp->rspheader, item );
    }
    return str;
}

/*
==========================================================
mjHttpRsp_New
    create new mjHttpRsp struct
==========================================================
*/
mjHttpRsp mjHttpRsp_New()
{
    mjHttpRsp rsp = ( mjHttpRsp ) calloc( 1, sizeof( struct mjHttpRsp ) );
    if ( !rsp ) {
        MJLOG_ERR( "calloc error" );
        return NULL;
    }

    rsp->rspheader = mjmap_new( 128 );
    if ( !rsp->rspheader ) {
        MJLOG_ERR( "mjmap_new error" );
        free( rsp );
        return NULL;
    }
    return rsp;
}

bool mjHttpRsp_Delete( mjHttpRsp rsp )
{
    if ( !rsp ) return false;
    
    if ( rsp->rspheader ) {
        mjmap_delete( rsp ->rspheader );
    }
    free( rsp );
    return true;
}

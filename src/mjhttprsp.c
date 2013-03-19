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

    mjStr tmp = mjStr_New();
    if ( !tmp ) {
        MJLOG_ERR( "mjStr_New error" );
        return false;
    }

    mjStr_CopyS( tmp, value );
    mjMap_Add( rsp->rspheader, name, tmp );
    mjStr_Delete( tmp );
    return true;
}

mjStr mjHttpRsp_HeaderToStr( mjHttpRsp rsp )
{
    mjStr str = mjStr_New();
    if ( !str ) {
        MJLOG_ERR( "mjStr_New error" );
        return NULL;
    }
    mjitem item = mjmap_GetNext( rsp->rspheader, NULL );
    while ( item ) {
        mjStr_Cat( str, item->key );
        mjStr_CatS( str, ": " );
        mjStr_Cat( str, item->value ); 
        mjStr_CatS( str, "\r\n" );
    
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

    rsp->rspheader = mjMap_New( 128 );
    if ( !rsp->rspheader ) {
        MJLOG_ERR( "mjMap_New error" );
        free( rsp );
        return NULL;
    }
    return rsp;
}

bool mjHttpRsp_Delete( mjHttpRsp rsp )
{
    if ( !rsp ) return false;
    
    if ( rsp->rspheader ) {
        mjMap_Delete( rsp ->rspheader );
    }
    free( rsp );
    return true;
}

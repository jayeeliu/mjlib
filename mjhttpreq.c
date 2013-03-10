#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "mjhttpreq.h"
#include "mjlog.h"

#define MAX_HEADER_LEN  20
#define MAX_FIELD_LEN   10

/*
=================================================================
mjHttpReq_New
    create new mjHttpReq struct
=================================================================
*/
mjHttpReq mjHttpReq_New( mjstr data )
{
    // sanity check
    if ( !data ) {
        MJLOG_ERR( "data is null" );
        return NULL;
    }
    // create mjHttpReq struct
    mjHttpReq request = ( mjHttpReq ) calloc( 1, sizeof( struct mjHttpReq ) );
    if ( !request ) {
        MJLOG_ERR( "mjHttpReq alloc error" );
        return NULL;
    }
    // get all header from data  
    mjStrList header = mjStrList_New();
    if ( !header ) {
        MJLOG_ERR( "mjStrList_New error" );
        goto failout1;
    }
    mjstr_split( data, "\r\n", header );
    // get filed from the first len 
    mjStrList field = mjStrList_New();
    if ( !field ) {
        MJLOG_ERR( "mjStrList_New error" );
        goto failout2;
    }
    mjstr_split( header->data[0], " ", field );
    // check cmd length
    if ( field->length < 2 ) {
        MJLOG_ERR( "parse header error" );
        goto failout3;
    }
    // get method type
    const char* method = field->data[0]->str;
    if ( !strcasecmp( method, "GET" ) ) { 
        request->methodType = GET_METHOD;
    } else if ( !strcasecmp( method, "POST" ) ) {
        request->methodType = POST_METHOD;
    } else {
        request->methodType = UNKNOWN_METHOD;
    }
    // get access location
    request->location = mjstr_new();
    mjstr_copy( request->location, field->data[1] );
    mjStrList_Clean( field );
    // parse other header
    request->reqHeader = mjMap_New(128);
    for ( int i = 1; i < header->length; i++ ) {
        if ( !header->data[i] ) break;
        mjstr_split( header->data[i], ":", field );
        if ( !field || field->length < 2 ) continue;
        mjmap_add( request->reqHeader, field->data[0]->str, field->data[1] );
        mjStrList_Clean( field );
    }
    // clean strlist
    mjStrList_Delete( field );
    mjStrList_Delete( header );
    return request;

failout3:
    mjStrList_Delete( field );
failout2:
    mjStrList_Delete( header );
failout1:
    free( request );
    return NULL;
}

/*
================================================
mjHttpReq_Delete
    delete mjHttpReq struct
================================================
*/
void mjHttpReq_Delete( mjHttpReq request )
{
    // sanity check
    if ( !request ) return;
    // free struct
    mjstr_delete( request->location );
    mjmap_delete( request->reqHeader );
    free( request );
    return;
}

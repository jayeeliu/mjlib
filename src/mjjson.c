#include <stdio.h>
#include <string.h>
#include "mjjson.h"
#include "libjpw/jpw.h"
#include "mjlog.h"

/* 
===========================================================================
mjJSON_Parse
    parse json string
    return  0: success
            400: json parse create error
            401: unable to parse token
            402: unable to get necessary field
============================================================================
*/
int mjJSON_Parse( const char *input, mjJSONStr strList[], mjJSONInt intList[] )
{
    // input null success
    if ( !input ) return 0;

    // create jpw_root object
    struct jpw_root* jpw = jpw_new();
    if( !jpw ) {
        MJLOG_ERR( "create json parser error" );
        return 400;
    }
    // parse json
    struct jpw_gen_t* fromobject = jpw_parse( jpw, input );
    if ( !fromobject ) {
        MJLOG_ERR( "unable to parse token" );
        jpw_delete( jpw );
        return 401;
    }
    // parse string variable
    for ( mjJSONStr* str = strList; str && str->name; str++ ) {
        struct jpw_gen_t* tmpjson;
        char* output = NULL;

        if ( !( tmpjson = jpw_object_get_item( jpw, fromobject, str->name ) )
            || !( output = jpw_string_get_str( jpw, tmpjson ) ) ) {
            // error happened
            if( !str->must ) {
                MJLOG_DEBUG( "parameter not set %s", str->name );
                continue;
            }
            MJLOG_ERR( "unable to get %s, parameter error", str->name );
            jpw_delete( jpw );
            return 402;
        }
        // set string value
        if ( str->val ) {
            int len = str->len > strlen( output ) ? strlen( output ) : str->len;
            memcpy( str->val, output, len );
        }
    }

    // parse int variable
    for ( mjJSONInt* intVal = intList; intVal && intVal->name; intVal++ ) {
        struct jpw_gen_t* tmpjson = jpw_object_get_item( jpw, fromobject, intVal->name );
        if ( !tmpjson ) {
            if ( !intVal->must ) {
                MJLOG_DEBUG( "parameter not set %s", intVal->name );
                continue;
            }
            MJLOG_ERR( "unable to get %s, parameter error", intVal->name );
            jpw_delete( jpw );
            return 402;
        }
        // set value
        if ( intVal->val ) {
            *( intVal->val ) = jpw_int_get_val( jpw, tmpjson );
        }
    }
    
    jpw_delete( jpw );
    return 0;
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include "mjopt.h"
#include "mjlog.h"
#include "mjio.h"

static LIST_HEAD(options);

/*
=====================================================
mjOpt_Set
    set value
=====================================================
*/
static void mjOpt_Set( mjOpt opt, char* value ) {
    // set default value 
    if ( opt->type == MJOPT_INT ) {
        if ( !value ) {
            *( int* )( opt->value ) = 0;
        } else {
            *( int* )( opt->value ) = atoi( value );
        }
    } else if ( opt->type == MJOPT_STR ) {
        if ( !value ) {
            strcpy( opt->value, "" );
        } else {
            strcpy( opt->value, value );
        }
    }
}

/*
===============================================================================
mjOpt_Define
    set option define
===============================================================================
*/
bool mjOpt_Define( char* section, char* key, int type, void* value,
            char* defaultValue ) {
    // sanity check
    if ( section && strlen( section ) >= MAX_SECTION_LEN ) {
        MJLOG_ERR( "section is too long" );
        return false;
    }
    if ( !key || strlen( key ) >= MAX_KEY_LEN ) {
        MJLOG_ERR( "key is too long" );
        return false;
    }
    if ( !value ) {
        MJLOG_ERR( "value is null" );
        return false;
    }
    if ( type != MJOPT_INT && type != MJOPT_STR ) {
        MJLOG_ERR( "type error" );
        return false;
    }
    // create mjOpt struct
    mjOpt opt = ( mjOpt ) calloc ( 1, sizeof( struct mjOpt ) );
    if ( !opt ) {
        MJLOG_ERR( "mjOpt alloc error" );
        return false;
    }
    // set section and key 
    if ( !section ) {
        strcpy( opt->section, "global" );
    } else {
        strcpy( opt->section, section );
    }
    strcpy( opt->key, key );
    // set type and value pointer
    opt->type   = type;
    opt->value  = value;
    // set default value 
    mjOpt_Set( opt, defaultValue );
    // add to options list
    INIT_LIST_HEAD( &opt->node );
    list_add_tail( &opt->node, &options );
    return true;
}

/*
===============================================================================
mjOpt_SetValue
    set option value
===============================================================================
*/
bool mjOpt_SetValue( char* section, char* key, char* value ) {
    // set default section
    char* global = "global";
    if ( !section ) section = global;
    // get and set value
    mjOpt entry = NULL;
    list_for_each_entry( entry, &options, node ) {
        if ( !strcmp( section, entry->section ) &&
            !strcmp( key, entry->key ) ) {
            // set value
            mjOpt_Set( entry, value );
            return true;
        }
    }
    // value no found
    return false;
}

/*
===============================================================================
mjOpt_ParseConf
    parse conf file
===============================================================================
*/
bool mjOpt_ParseConf( const char* fileName ) {
    char section[MAX_SECTION_LEN];
    char key[MAX_KEY_LEN];
    char value[MAX_VALUE_LEN];

    mjIO io = mjIO_New( fileName );
    if ( !io ) {
        MJLOG_ERR( "mjio alloc error" );
        return false;
    }
    mjStr line = mjStr_New();
    if ( !line ) {
        MJLOG_ERR( "mjStr_New error" );
        mjIO_Delete( io );
        return false;
    }
    // set default section
    strcpy( section, "global" );
    while ( 1 ) {
        // get one line from file
        int ret = mjIO_ReadLine( io, line );
        if ( ret <= 0 ) break;
        mjStr_Strim( line );
        // ignore empty line
        if ( line->length == 0 ) continue;
        // ignore comment line
        if ( line->data[0] == '#' ) continue;
        // section line, get section
        if ( line->data[0] == '[' && 
            line->data[line->length-1] == ']' ) {
            mjStr_Consume( line, 1 );
            mjStr_RConsume( line, 1 );
            mjStr_Strim( line );
            // section can't be null
            if ( line->length == 0 ) {
                MJLOG_ERR( "section is null" );
                mjIO_Delete( io );
                mjStr_Delete( line );
                return false;
            }
            strcpy( section, line->data );
            continue;
        }
        // split key and value
        mjStrList strList = mjStrList_New();
        mjStr_Split( line, "=", strList );
        if ( strList->length != 2 ) {
            MJLOG_ERR( "conf error" );
            mjStrList_Delete( strList );
            mjStr_Delete( line );
            mjIO_Delete( io );
            return false;
        }
        mjStr keyStr = mjStrList_Get( strList, 0 );
        mjStr valueStr = mjStrList_Get( strList, 1 );
        mjStr_Strim( keyStr );
        mjStr_Strim( valueStr );
        strcpy( key, keyStr->data );
        strcpy( value, valueStr->data );
        mjStrList_Delete( strList );
        // set option value
        mjOpt_SetValue( section, key, value );
    }

    mjStr_Delete( line );
    mjIO_Delete( io );
    return true;
}

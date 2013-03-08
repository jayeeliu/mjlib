#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mjopt.h"
#include "mjlog.h"

static LIST_HEAD(options);

/*
=====================================================
mjOpt_Set
    set value
=====================================================
*/
static void mjOpt_Set( mjOpt opt, char* value )
{
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
===================================================================
mjOpt_Define
    set option define
===================================================================
*/
bool mjOpt_Define( char* section, char* key, int type, void* value,
            char* defaultValue, char* cmdKey, int cmdKeyValue, char* helpString )
{
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
    mjOpt opt = ( mjOpt ) calloc( 1, sizeof( struct mjOpt ) );
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
    // set cmdKey
    if ( !cmdKey ) {
        strcpy( opt->cmdKey, "" );
        opt->cmdKeyValue    = 0;
    } else {
        strcpy( opt->cmdKey, cmdKey );
        opt->cmdKeyValue    = cmdKeyValue == 0 ? 0 : 1;
    }
    // set help string
    opt->helpString = helpString;
    // add to options list
    INIT_LIST_HEAD( &opt->node );
    list_add_tail( &opt->node, &options );

    return true;
}

/*
==============================================================
mjOpt_SetValue
    set option value
==============================================================
*/
bool mjOpt_SetValue( char* section, char* key, char* value )
{
    char* global = "global";
    if ( !section ) section = global;

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

bool mjOpt_ParseConf( const char* fileName )
{
    return true;
}

bool mjOpt_ParseCmd( int argc, char* argv[] )
{
    for( int i = 1; i < argc; i++ ) {
        char* tmp = argv[i];
        if ( tmp[0] == '-' ) {
            tmp++;
            if ( strlen( tmp ) == 0 ) {
                printf( "arg false\n" );
                return false; 
            }

            mjOpt entry = NULL;
            list_for_each_entry( entry, &options, node ) {
                // no need cmd, continue
                if ( !strcmp( entry->cmdKey, "" ) ) continue;
                // match cmd
                if ( !strcmp( entry->cmdKey, tmp ) ) {
                    if ( entry->cmdKeyValue == 1 ) {  
                        tmp = argv[i + 1];
                        mjOpt_Set( entry, tmp );    
                        i++;
                    }
                }
            }
        }
    }
    return true;
}

void mjOpt_HelpString()
{
    mjOpt entry = NULL;
    list_for_each_entry(entry, &options, node) {
        if ( strcmp( entry->cmdKey, "" ) ) {
            if ( !entry->cmdKeyValue ) {
                printf( "-%s: %s\n", entry->cmdKey, entry->helpString );
            } else {
                printf( "-%s <value>: %s\n", entry->cmdKey, entry->helpString );
            }
        }
    }
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mjlog.h"
#include "mjreg.h"

#define MAXLEN  20

bool mjReg_Search( mjReg reg, char* string, mjStrList result )
{
    // call regexec to search string
    regmatch_t pm[MAXLEN];
    int ret = regexec( &reg->preg, string, MAXLEN, pm, 0 );
    if ( ret != 0 ) return false;
    // no result should be return   
    if ( !result ) return true;
    // copy data 
    for ( int i = 0; i < MAXLEN && pm[i].rm_so != -1; i++ ) {
        mjStrList_AddB( result, string + pm[i].rm_so, pm[i].rm_eo - pm[i].rm_so);
    }

    return true;
}

/*
============================================================
mjReg_new
    create new mjReg struct
    return  NULL -- fail,
            other -- success
============================================================
*/
mjReg mjReg_New( const char* regex )
{
    // create mjReg struct
    mjReg reg = ( mjReg ) calloc( 1, sizeof( struct mjReg ) );
    if ( !reg ) {
        MJLOG_ERR( "mjReg calloc error" );
        return NULL;
    }
    // init reg
    int ret = regcomp( &reg->preg, regex, REG_EXTENDED );
    if ( ret != 0 ) {
        MJLOG_ERR( "regcomp error" );
        free( reg );
        return NULL;
    }
    return reg;
}

/*
======================================
mjReg_delete
    delete mjReg struct
    no return
======================================
*/
void mjReg_Delete( mjReg reg )
{
    if ( !reg ) return;

    regfree( &reg->preg );
    free( reg );
    return;
}

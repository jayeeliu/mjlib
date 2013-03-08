#include <stdlib.h>
#include <string.h>
#include "mjlog.h"
#include "mjstr.h"

/**
 * ready size for mjstring
 */
static bool mjstr_ready( mjstr x, unsigned int n )
{
    // string must not be null
    if ( !x ) return false;

    unsigned int i = x->total; 
    if ( n <= i ) return true; // have enough size, return
    
    /* need to realloc */
    x->total = 30 + n + ( n >> 3 ); /* new size to alloc */
    char *tmpstr = ( char* ) realloc( x->str, x->total ); /* realloc new size*/
    if ( !tmpstr ) {  /* alloc failed */
        x->total = i; /* restore total size */
        return false;
    }

    x->str = tmpstr; /* we get new memory */
    x->str[x->length] = 0; /* set byte to zero */
    return true;
}

static bool mjstr_readyplus( mjstr x, unsigned int n )
{
    return mjstr_ready( x, x->length + n );
}

bool mjstr_copyb(mjstr sa, const char* s, unsigned int n)
{
    if ( !mjstr_ready( sa, n + 1 ) ) return false; /* extend if needed*/
    memcpy( sa->str, s, n );    /* copy string */
    sa->length  = n;             /* set new used size */
    sa->str[n]  = '\0';          /* set byte to zero */
    return true;
}

bool mjstr_copy( mjstr sato, mjstr safrom )
{
    if ( !sato || !safrom ) return false;
    return mjstr_copyb( sato, safrom->str, safrom->length );
}

bool mjstr_copys( mjstr sa, const char* s )
{
    if ( !sa || !s ) return false;
    return mjstr_copyb( sa, s, strlen( s ) );
}

bool mjstr_catb(mjstr sa, char *s, unsigned int n)
{
  if (!sa->str) return mjstr_copyb(sa, s, n);       /* sa is null, copy from s */
  if (!mjstr_readyplus(sa, n + 1)) return false;    /* extend if needed */
  memcpy(sa->str + sa->length, s, n);               /* copy string */
  sa->length += n;
  sa->str[sa->length] = '\0';                       /* set byte to zero */

  return true;
}

bool mjstr_cat(mjstr sato, mjstr safrom)
{
    return mjstr_catb(sato, safrom->str, safrom->length);
}

bool mjstr_cats(mjstr sa, char *s)
{
    return mjstr_catb(sa, s, strlen(s));
}

/*
=================================================
mjstr_consume
    consume len string
=================================================
*/
int mjstr_consume( mjstr x, unsigned int len )
{
    // sanity check
    if ( len <= 0 ) return 0;

    int ret;
    if (len >= x->length) {
        ret = x->length;
        x->length = 0;
        x->str[x->length] = 0;
        return ret;
    }
    memmove(x->str, x->str + len, x->length - len);
    x->length -= len;
    x->str[x->length] = 0;
    return len;
}

int mjstr_rconsume( mjstr x, unsigned int len )
{
    // sanity check
    if ( len <= 0 ) return 0;
    // adjust length
    if ( x->length < len ) {
        x->length = 0;
    } else {
        x->length -= len;
    }
    x->str[x->length] = 0;

    return len;
}

int mjstr_search(mjstr x, const char *split)
{
    if (x == NULL || x->str == NULL || split == NULL) return -1;
    if (x->length == 0) return -1;

    char *point = strstr(x->str, split);
    if (point == NULL) return -1;
   
    return point - x->str;
}

void mjstr_ltrim( mjstr x )
{
    if ( !x ) return;
    
    int pos;
    for( pos = 0; pos < x->length; pos++ ) {
        if ( x->str[pos] == '\t' || x->str[pos] == ' ' ||
            x->str[pos] == '\r' || x->str[pos] == '\n' ) continue;
        break;
    }
    mjstr_consume( x, pos );
}

void mjstr_rtrim( mjstr x )
{
    if ( !x ) return;
    int pos;
    for( pos = x->length - 1; pos >= 0; pos-- ) {
        if ( x->str[pos] == '\t' || x->str[pos] == ' ' ||
            x->str[pos] == '\r' || x->str[pos] == '\n' ) continue;
        break;
    }
    x->length = pos + 1;
    x->str[x->length] = 0;
}

void mjstr_strim( mjstr x )
{
    mjstr_ltrim( x );
    mjstr_rtrim( x );
}

/*
==================================================================
mjstr_split
    split mjstr into mjStrList
    return: true --- success; false --- failed;
==================================================================
*/
bool mjstr_split( mjstr x, const char* split, mjStrList strList )
{
    // santy check
    if ( !x || !strList ) return false;

    int start   = 0;
    while ( start < x->length ) {
        char* point = strstr( x->str + start, split );
        if ( !point ) break;
        
        if ( point - x->str != start ) {
            mjStrList_AddB( strList, x->str + start, point - x->str - start );
        }
        start = point - x->str + strlen( split );
    }
    mjStrList_AddB( strList, x->str + start, x->length - start );

    return true;
}

int mjstr_cmp(mjstr str1, mjstr str2)
{
    if (str1 == NULL && str2 == NULL) return 0;
    if (str1 == NULL && str2 != NULL) return -1;
    if (str1 != NULL && str2 == NULL) return 1;

    int minlen = (str1->length > str2->length)?str2->length:str1->length;
    int ret = memcmp(str1->str, str2->str, minlen);
    if (ret != 0) return ret;
   
    if (str1->length == str2->length) return 0;
    if (str1->length > str2->length) {
        return 1;
    } else {
        return -1;
    }
}

/*
=========================================================
mjstr_new 
    create new mjstr
=========================================================
*/
mjstr mjstr_new()
{
    mjstr ret = ( mjstr ) calloc ( 1, sizeof( struct mjstr ) );
    if ( !ret ) return NULL;

    ret->str    = NULL;
    ret->length = 0;        /* no used, now */
    ret->total  = 0;        /* total size */
    
    return ret;
}

/*
===================================
mjstr_delete
    free mjstr
===================================
*/
void mjstr_delete( mjstr x )
{
    if ( !x ) return;

    free( x->str );
    free( x );
}

/*
======================================================
mjStrList_Add
    add mjstr to strList
======================================================
*/
bool mjStrList_Add( mjStrList strList, mjstr str )
{
    return mjStrList_AddB( strList, str->str, str->length );
}

/*
======================================================
mjStrList_AddS
    add str to strList
======================================================
*/
bool mjStrList_AddS( mjStrList strList, char* str )
{
    return mjStrList_AddB( strList, str, strlen( str ) );
}

/*
======================================================
mjStrList_AddS
    add new string in strList
======================================================
*/
bool mjStrList_AddB( mjStrList strList, char* str, int len )
{
    if ( !strList ) {
        MJLOG_ERR( "sanity check error" );
        return false;
    }
    // strList is full, realloc
    if ( strList->length == strList->total ) {
        // realloc strList
        strList->total += 12;
        mjstr* tmpList = ( mjstr* ) realloc( strList->data, 
                                strList->total * sizeof( mjstr ) );
        if ( !tmpList ) {
            MJLOG_ERR( "realloc error" );
            strList->total -= 12;
            return false;
        }
        strList->data = tmpList;
        // init new alloc area to NULL 
        for ( int i = strList->length; i < strList->total; i++ ) {
            strList->data[i] = NULL;
        }
    } 
    // copy string
    if ( !strList->data[strList->length] ) {
        mjstr tmpStr = mjstr_new();
        if ( !tmpStr ) {
            MJLOG_ERR( "mjstr_new error" );
            return false;
        }
        strList->data[strList->length] = tmpStr;
    }
    mjstr_copyb( strList->data[strList->length], str , len);

    strList->length++;
    return 0;
}

/*
==========================================================
mjStrList_Get
    get idx in strList
==========================================================
*/
mjstr mjStrList_Get( mjStrList strList, unsigned int idx )
{
    if ( !strList || idx >= strList->length ) {
        MJLOG_ERR( "sanity check error" );
        return NULL;
    }
    return strList->data[idx];
}

/*
===========================================
mjStrList_Clean
    clean mjstrlist, length set to zero
===========================================
*/
bool mjStrList_Clean( mjStrList strList )
{
    if ( !strList ) {
        MJLOG_ERR( "sanity check error" );
        return false;
    }
    strList->length = 0;    
    return true;
}

/*
=========================================================
mjStrList_New
    alloc new mjStrList struct
=========================================================
*/
mjStrList mjStrList_New() 
{
    mjStrList strList = ( mjStrList ) calloc( 1, sizeof( struct mjStrList ) );
    if ( !strList ) {
        MJLOG_ERR( "mjStrList create error" );
        return NULL;
    }
    strList->length     = 0;
    strList->total      = 0;
    strList->data       = NULL;
    return strList;
}

/*
=======================================================
mjStrList_Delete
    delete mjStrList
=======================================================
*/
bool mjStrList_Delete( mjStrList strList )
{
    if ( !strList ) return false;
    // clean strlist
    for ( int i = 0; i < strList->total; i++ ) {
        if ( strList->data[i] ) {
            mjstr_delete( strList->data[i] );
        }
    }
    free( strList->data );
    free( strList );
    return true;
}

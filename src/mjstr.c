#include <stdlib.h>
#include <string.h>
#include "mjlog.h"
#include "mjstr.h"

/*
=====================================================
mjStr_Ready
    alloc enough size for mjString
=====================================================
*/
static bool mjStr_Ready( mjStr x, unsigned int n )
{
    // string must not be null
    if ( !x ) return false;

    unsigned int i = x->total; 
    if ( n <= i ) return true; // have enough size, return
    
    //  need to realloc
    x->total = 30 + n + ( n >> 3 ); //  new size to alloc
    char *tmpstr = ( char* ) realloc( x->data, x->total ); // realloc new size
    if ( !tmpstr ) {  // alloc failed 
        x->total = i; // restore total size
        return false;
    }

    x->data = tmpstr; // we get new memory 
    x->data[x->length] = 0; // set byte to zero
    return true;
}

static bool mjStr_ReadyPlus( mjStr x, unsigned int n )
{
    return mjStr_Ready( x, x->length + n );
}

bool mjStr_CopyB(mjStr sa, const char* s, unsigned int n)
{
    if ( !mjStr_Ready( sa, n + 1 ) ) return false; /* extend if needed*/
    memcpy( sa->data, s, n );    /* copy string */
    sa->length  = n;             /* set new used size */
    sa->data[n]  = '\0';          /* set byte to zero */
    return true;
}

bool mjStr_Copy( mjStr sato, mjStr safrom )
{
    if ( !sato || !safrom ) return false;
    return mjStr_CopyB( sato, safrom->data, safrom->length );
}

bool mjStr_CopyS( mjStr sa, const char* s )
{
    if ( !sa || !s ) return false;
    return mjStr_CopyB( sa, s, strlen( s ) );
}

bool mjStr_CatB(mjStr sa, const char *s, unsigned int n)
{
  if ( !sa->data ) return mjStr_CopyB( sa, s, n );       /* sa is null, copy from s */
  if ( !mjStr_ReadyPlus(sa, n + 1)) return false;    /* extend if needed */
  memcpy(sa->data + sa->length, s, n);               /* copy string */
  sa->length += n;
  sa->data[sa->length] = '\0';                       /* set byte to zero */

  return true;
}

bool mjStr_Cat(mjStr sato, mjStr safrom)
{
    return mjStr_CatB(sato, safrom->data, safrom->length);
}

bool mjStr_CatS(mjStr sa, const char *s)
{
    return mjStr_CatB(sa, s, strlen(s));
}

/*
=================================================
mjStr_Consume
    consume len string
=================================================
*/
int mjStr_Consume( mjStr x, unsigned int len )
{
    // sanity check
    if ( len <= 0 ) return 0;

    int ret;
    if (len >= x->length) {
        ret = x->length;
        x->length = 0;
        x->data[x->length] = 0;
        return ret;
    }
    memmove(x->data, x->data + len, x->length - len);
    x->length -= len;
    x->data[x->length] = 0;
    return len;
}

int mjStr_RConsume( mjStr x, unsigned int len )
{
    // sanity check
    if ( len <= 0 ) return 0;
    // adjust length
    if ( x->length < len ) {
        x->length = 0;
    } else {
        x->length -= len;
    }
    x->data[x->length] = 0;

    return len;
}

/*
====================================================
mjStr_Search
    search string in x
    return startpositon in x
            -1 for no found or error
====================================================
*/
int mjStr_Search( mjStr x, const char *split )
{
    // sanity check
    if ( x == NULL || x->data == NULL || split == NULL ) return -1;
    if ( x->length == 0 ) return -1;

    char *point = strstr( x->data, split );
    if ( point == NULL ) return -1;
   
    return point - x->data;
}

void mjStr_LStrim( mjStr x )
{
    if ( !x ) return;
    
    int pos;
    for( pos = 0; pos < x->length; pos++ ) {
        if ( x->data[pos] == '\t' || x->data[pos] == ' ' ||
            x->data[pos] == '\r' || x->data[pos] == '\n' ) continue;
        break;
    }
    mjStr_Consume( x, pos );
}

void mjStr_RStrim( mjStr x )
{
    if ( !x ) return;
    int pos;
    for( pos = x->length - 1; pos >= 0; pos-- ) {
        if ( x->data[pos] == '\t' || x->data[pos] == ' ' ||
            x->data[pos] == '\r' || x->data[pos] == '\n' ) continue;
        break;
    }
    x->length = pos + 1;
    x->data[x->length] = 0;
}

void mjStr_Strim( mjStr x )
{
    mjStr_LStrim( x );
    mjStr_RStrim( x );
}

/*
==================================================================
mjStr_Split
    split mjStr into mjStrList
    return: true --- success; false --- failed;
==================================================================
*/
bool mjStr_Split( mjStr x, const char* split, mjStrList strList )
{
    // santy check
    if ( !x || !strList ) return false;

    int start   = 0;
    while ( start < x->length ) {
        char* point = strstr( x->data + start, split );
        if ( !point ) break;
        
        if ( point - x->data != start ) {
            mjStrList_AddB( strList, x->data + start, point - x->data - start );
        }
        start = point - x->data + strlen( split );
    }
    mjStrList_AddB( strList, x->data + start, x->length - start );

    return true;
}

/*
======================================================
mjStr_Cmp
    compare two mjStr
======================================================
*/
int mjStr_Cmp(mjStr str1, mjStr str2)
{
    if (str1 == NULL && str2 == NULL) return 0;
    if (str1 == NULL && str2 != NULL) return -1;
    if (str1 != NULL && str2 == NULL) return 1;

    int minlen = (str1->length > str2->length)?str2->length:str1->length;
    int ret = memcmp(str1->data, str2->data, minlen);
    if (ret != 0) return ret;
   
    if (str1->length == str2->length) return 0;
    if (str1->length > str2->length) {
        return 1;
    } else {
        return -1;
    }
}

bool mjStr_ToLower( mjStr str )
{
    if ( !str ) {
        MJLOG_ERR( "str is null" );
        return false;
    }

    for( int i = 0; i < str->length; i++) {
        if ( str->data[i] >='A' && str->data[i] <= 'Z' ) {
            str->data[i] += 32;
        }
    }
    return true;
}

/*
=========================================================
mjStr_Capitablize
    change mjstr to capitable
=========================================================
*/
bool mjStr_ToUpper( mjStr str )
{
    if ( !str ) {
        MJLOG_ERR( "str is Null" );
        return false;
    }

    for( int i = 0; i < str->length; i++ ) {
        if ( str->data[i] >= 'a' && str->data[i] <= 'z' ) {
            str->data[i] -= 32;
        }
    }
    return true;
}

/*
===============================================================================
mjStr_Init
    init mjstr
===============================================================================
*/
bool mjStr_Init( mjStr str ) {
    str->data   = NULL;
    str->length = 0;
    str->total  = 0;
    return true;
}

/*
===============================================================================
mjStr_New 
    create new mjStr
===============================================================================
*/
mjStr mjStr_New() {
    mjStr str = ( mjStr ) calloc ( 1, sizeof( struct mjStr ) );
    if ( !str ) return NULL;
    return str;
}

/*
===============================================================================
mjStr_Delete
    free mjStr
===============================================================================
*/
bool mjStr_Delete( mjStr str ) {
    if ( !str ) return false;
    free( str->data );
    free( str );
    return true;
}

static bool mjStrList_Ready( mjStrList strList, unsigned int n )
{
    if ( !strList ) return false;

    unsigned int i = strList->total;
    if ( n <= i ) return true;

    strList->total = 30 + n + ( n >> 3 );

    mjStr* tmp = ( mjStr* ) realloc ( strList->data, 
                            strList->total * sizeof ( mjStr ) );
    if ( !tmp ) {
        MJLOG_ERR( "realloc error" );
        strList->total = i;
        return false;
    }
    strList->data = tmp;

    for ( int i = strList->length; i < strList->total; i++ ) {
        strList->data[i] = 0;
    }

    return true;
}

static bool mjStrList_ReadyPlus( mjStrList strList, unsigned int n )
{
    return mjStrList_Ready( strList, strList->length + n );
}
 
/*
======================================================
mjStrList_Add
    add mjStr to strList
======================================================
*/
bool mjStrList_Add( mjStrList strList, mjStr str )
{
    return mjStrList_AddB( strList, str->data, str->length );
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
    // alloc enough space
    if ( !mjStrList_ReadyPlus( strList, 1 ) ) {
        MJLOG_ERR( "mjStrList Ready Error" );
        return false;
    }
    // copy string
    if ( !strList->data[strList->length] ) {
        mjStr tmpStr = mjStr_New();
        if ( !tmpStr ) {
            MJLOG_ERR( "mjStr_New error" );
            return false;
        }
        strList->data[strList->length] = tmpStr;
    }
    mjStr_CopyB( strList->data[strList->length], str , len );

    strList->length++;
    return 0;
}

/*
==========================================================
mjStrList_Get
    get idx in strList
==========================================================
*/
mjStr mjStrList_Get( mjStrList strList, unsigned int idx )
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
    clean mjStrlist, length set to zero
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
            mjStr_Delete( strList->data[i] );
        }
    }
    free( strList->data );
    free( strList );
    return true;
}

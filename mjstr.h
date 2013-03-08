#ifndef _MJSTR_H
#define _MJSTR_H

#include <stdbool.h>

// mjstr struct 
struct mjstr {
    unsigned int    length;         // used length 
    unsigned int    total;          // total length
    char*           str;            // point to the string
};
typedef struct mjstr* mjstr;

// mjstrlist struct
struct mjStrList {
    unsigned int    length;         // used length
    unsigned int    total;          // count of mjstr
    mjstr*          data;           // mjstr list
};
typedef struct mjStrList* mjStrList;

// function for mjstr
extern bool     mjstr_copy( mjstr sato, mjstr safrom );
extern bool     mjstr_copys( mjstr sa, const char *s );
extern bool     mjstr_copyb( mjstr sa, const char *s, unsigned int n );
extern bool     mjstr_cat( mjstr sato, mjstr safrom );
extern bool     mjstr_cats( mjstr sa, char *s );
extern bool     mjstr_catb( mjstr sa, char *s, unsigned int n );
extern int      mjstr_consume( mjstr x, unsigned int len );
extern int      mjstr_rconsume( mjstr x, unsigned int len );
extern int      mjstr_search( mjstr x, const char *split );
extern int      mjstr_cmp( mjstr str1, mjstr str2 );
extern bool     mjstr_split( mjstr x, const char* split, mjStrList strList );
extern void     mjstr_strim( mjstr x );
extern void     mjstr_lstrim( mjstr x );
extern void     mjstr_rstrim( mjstr x );

extern mjstr    mjstr_new();
extern void     mjstr_delete( mjstr x );

// function for mjStrList
extern bool         mjStrList_Add( mjStrList strList, mjstr str );
extern bool         mjStrList_AddS( mjStrList strList, char* str );
extern bool         mjStrList_AddB( mjStrList strList, char* str , int len );
extern mjstr        mjStrList_Get( mjStrList strList, unsigned int idx );
extern bool         mjStrList_Clean( mjStrList strList );

extern mjStrList    mjStrList_New();
extern bool         mjStrList_Delete( mjStrList strList );

#endif

#ifndef _MJSTR_H
#define _MJSTR_H

#include <stdbool.h>

// mjstr struct 
struct mjstr {
  unsigned int  length;         // used length 
  unsigned int  total;          // total length
  char*         data;           // point to the string
};
typedef struct mjstr* mjstr;

// mjstrlist struct
struct mjstrlist {
  unsigned int  length;         // used length
  unsigned int  total;          // count of mjstr
  mjstr*        data;           // mjstr list
};
typedef struct mjstrlist* mjstrlist;

// function for mjstr
extern bool   mjstr_copy(mjstr sato, mjstr safrom);
extern bool   mjstr_copys(mjstr sa, const char *s);
extern bool   mjstr_copyb(mjstr sa, const char *s, unsigned int n);
extern bool   mjstr_cat(mjstr sato, mjstr safrom);
extern bool   mjstr_cats(mjstr sa, const char *s);
extern bool   mjstr_catb(mjstr sa, const char *s, unsigned int n);
extern int    mjstr_consume(mjstr str, unsigned int len);
extern int    mjstr_rconsume(mjstr str, unsigned int len);
extern int    mjstr_search( mjstr x, const char *split );
extern int    mjstr_cmp(mjstr str1, mjstr str2);
extern bool   mjstr_split(mjstr str, const char* split, mjstrlist strList);
extern void   mjstr_strim(mjstr str);
extern void   mjstr_lstrim(mjstr str);
extern void   mjstr_rstrim(mjstr str);
extern bool   mjstr_tolower(mjstr str);
extern bool   mjstr_toupper(mjstr str);

extern bool   mjstr_init(mjstr str);
extern bool   mjstr_deinit(mjstr str);
extern mjstr  mjstr_new();
extern bool   mjstr_delete(mjstr x);
// function for mjstrlist
extern bool       mjstrlist_add(mjstrlist strList, mjstr str);
extern bool       mjstrlist_adds(mjstrlist strList, char* str);
extern bool       mjstrlist_addb(mjstrlist strList, char* str , int len);
extern mjstr      mjstrlist_get(mjstrlist strList, unsigned int idx);
extern bool       mjstrlist_clean(mjstrlist strList);

extern mjstrlist  mjstrlist_new();
extern bool       mjstrlist_delete(mjstrlist strList);

#endif

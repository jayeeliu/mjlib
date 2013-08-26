#ifndef _MJSTR_H
#define _MJSTR_H

#include <stdbool.h>

// mjstr struct 
struct mjstr {
  unsigned int  _start;         // start pos in data
  unsigned int  _length;        // used length 
  unsigned int  _total;         // total length
  char*         _data;          // point to the string
  char          _data_buf[0];   // default data buffer
};
typedef struct mjstr* mjstr;

// mjstrlist struct
struct mjstrlist {
  unsigned int  _length;         // used length
  unsigned int  _total;          // count of mjstr
  mjstr*        _data;           // mjstr list
};
typedef struct mjstrlist* mjstrlist;

// function for mjstr
extern bool   mjstr_copy(mjstr str_to, mjstr str_from);
extern bool   mjstr_copys(mjstr str_to, const char* src);
extern bool   mjstr_copyb(mjstr str_to, const char* src, unsigned int len);
extern bool   mjstr_cat(mjstr str_to, mjstr str_from);
extern bool   mjstr_cats(mjstr str_to, const char* src);
extern bool   mjstr_catb(mjstr str_to, const char* src, unsigned int len);
extern bool   mjstr_clean(mjstr str);
extern int    mjstr_cmp(mjstr str1, mjstr str2);
extern int    mjstr_get_length(mjstr str);
extern int    mjstr_consume(mjstr str, unsigned int len);
extern int    mjstr_rconsume(mjstr str, unsigned int len);
extern int    mjstr_search(mjstr str, const char* split);
extern bool   mjstr_split(mjstr str, const char* split, mjstrlist str_list);
extern void   mjstr_strim(mjstr str);
extern void   mjstr_lstrim(mjstr str);
extern void   mjstr_rstrim(mjstr str);
extern char*  mjstr_tochar(mjstr str);
extern bool   mjstr_tolower(mjstr str);
extern bool   mjstr_toupper(mjstr str);

extern mjstr  mjstr_new(unsigned int default_len);
extern bool   mjstr_delete(mjstr str);
// function for mjstrlist
extern bool       mjstrlist_add(mjstrlist str_list, mjstr str);
extern bool       mjstrlist_adds(mjstrlist str_list, char* str);
extern bool       mjstrlist_addb(mjstrlist str_list, char* str , int len);
extern mjstr      mjstrlist_get(mjstrlist str_list, unsigned int idx);
extern bool       mjstrlist_clean(mjstrlist str_list);

extern mjstrlist  mjstrlist_new();
extern bool       mjstrlist_delete(mjstrlist str_list);

#endif

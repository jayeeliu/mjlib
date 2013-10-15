#ifndef _MJSTR_H
#define _MJSTR_H

#include <stdbool.h>
#include <string.h>

// mjstr struct 
struct mjstr {
  char*         data;
  unsigned int  length;         // used length, string length
  unsigned int  _total;         // total length
  char*         _data_start;    // point to the string
  char          _data_buf[0];   // default data buffer
};
typedef struct mjstr* mjstr;

// mjstrlist struct
struct mjstrlist {
  mjstr*        data;           // mjstr list
  unsigned int  length;         // used length
  unsigned int  _total;         // count of mjstr
};
typedef struct mjstrlist* mjstrlist;


// function for mjstr
extern bool   mjstr_catb(mjstr str_to, const char* src, unsigned int len);
extern bool   mjstr_copyb(mjstr str_to, const char* src, unsigned int len);
extern int    mjstr_cmp(mjstr str1, mjstr str2);
extern int    mjstr_consume(mjstr str, unsigned int len);
extern int    mjstr_rconsume(mjstr str, unsigned int len);
extern int    mjstr_search(mjstr str, const char* split);
extern bool   mjstr_split(mjstr str, const char* split, mjstrlist str_list);
extern void   mjstr_lstrim(mjstr str);
extern void   mjstr_rstrim(mjstr str);
extern bool   mjstr_tolower(mjstr str);
extern bool   mjstr_toupper(mjstr str);
extern mjstr  mjstr_new(unsigned int default_len);
extern bool   mjstr_delete(mjstr str);

static inline bool mjstr_cat(mjstr str_to, mjstr str_from) {
  if (!str_to || !str_from) return false;
  return mjstr_catb(str_to, str_from->data, str_from->length);
}

static inline bool mjstr_cats(mjstr str, const char* src) {
  if (!str || !src) return false;
  return mjstr_catb(str, src, strlen(src));
}

static inline bool mjstr_copy(mjstr str_to, mjstr str_from) {
  if (!str_to || !str_from) return false;
  return mjstr_copyb(str_to, str_from->data, str_from->length);
}

static inline bool mjstr_copys(mjstr str, const char* src) {
  if (!str || !src) return false;
  return mjstr_copyb(str, src, strlen(src));
}

static inline void mjstr_clean(mjstr str) {
  if (!str) return;
  str->data = str->_data_start;
  str->length = 0;
  str->data[str->length] = 0;
}

static inline void mjstr_strim(mjstr x) {
  mjstr_lstrim(x);
  mjstr_rstrim(x);
}


// function for mjstrlist
extern bool       mjstrlist_add(mjstrlist str_list, mjstr str);
extern bool       mjstrlist_adds(mjstrlist str_list, char* str);
extern bool       mjstrlist_addb(mjstrlist str_list, char* str , int len);
extern mjstr      mjstrlist_get(mjstrlist str_list, unsigned int idx);
extern bool       mjstrlist_clean(mjstrlist str_list);

extern mjstrlist  mjstrlist_new();
extern bool       mjstrlist_delete(mjstrlist str_list);

#endif

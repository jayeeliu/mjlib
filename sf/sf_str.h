#ifndef _MJSTR_H
#define _MJSTR_H

#include <stdbool.h>
#include <string.h>

// mjstr struct 
struct mjstr {
  char*     data;
  unsigned  len;            // used length, string length
  unsigned  _total;         // total length
  char*     _data_start;    // point to the string
  char      _data_buf[0];   // default data buffer
};
typedef struct mjstr* mjstr;

// mjslist struct
struct mjslist {
  mjstr*    data;           // mjstr list
  unsigned  len;            // used length
  unsigned  _total;         // count of mjstr
};
typedef struct mjslist* mjslist;

// function for mjstr
extern bool   mjstr_catb(mjstr str, const char* src, unsigned int len);
extern bool   mjstr_copyb(mjstr str, const char* src, unsigned int len);
extern int    mjstr_cmp(mjstr str1, mjstr str2);
extern int    mjstr_consume(mjstr str, unsigned int len);
extern int    mjstr_rconsume(mjstr str, unsigned int len);
extern int    mjstr_search(mjstr str, const char* split);
extern bool   mjstr_split(mjstr str, const char* split, mjslist slist);
extern void   mjstr_lstrim(mjstr str);
extern void   mjstr_rstrim(mjstr str);
extern bool   mjstr_tolower(mjstr str);
extern bool   mjstr_toupper(mjstr str);
extern mjstr  mjstr_new(unsigned int size);
extern bool   mjstr_delete(mjstr str);

static inline bool mjstr_cat(mjstr str, mjstr src) {
  if (!str || !src) return false;
  return mjstr_catb(str, src->data, src->len);
}

static inline bool mjstr_cats(mjstr str, const char* src) {
  if (!str || !src) return false;
  return mjstr_catb(str, src, strlen(src));
}

static inline bool mjstr_copy(mjstr str, mjstr src) {
  if (!str || !src) return false;
  return mjstr_copyb(str, src->data, src->len);
}

static inline bool mjstr_copys(mjstr str, const char* src) {
  if (!str || !src) return false;
  return mjstr_copyb(str, src, strlen(src));
}

static inline void mjstr_clean(mjstr str) {
  if (!str) return;
  str->data = str->_data_start;
  str->len = 0;
  str->data[str->len] = 0;
}

static inline void mjstr_strim(mjstr str) {
  mjstr_lstrim(str);
  mjstr_rstrim(str);
}


// function for mjslist
extern bool     mjslist_addb(mjslist slist, char* str , int len);
extern mjslist  mjslist_new();
extern bool     mjslist_delete(mjslist slist);


static inline bool mjslist_add(mjslist slist, mjstr str) {
  if (!slist || !str) return false;
  return mjslist_addb(slist, str->data, str->len);
}

static inline bool mjslist_adds(mjslist slist, char* str) {
  if (!slist || !str) return false;
  return mjslist_addb(slist, str, strlen(str));
}

static inline mjstr mjslist_get(mjslist slist, unsigned int idx) {
  if (!slist || idx >= slist->len) return NULL;
  return slist->data[idx];
}

static inline void mjslist_clean(mjslist slist) {
  if (!slist) return;
  slist->len = 0;
}

#endif

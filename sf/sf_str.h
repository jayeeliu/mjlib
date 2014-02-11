#ifndef __SF_STR_H
#define __SF_STR_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct sf_str {
  char*     data;
  unsigned  len;      // used length, string length
  unsigned  _size;    // total size
  char*     _start;   // point to the real buffer
  char      _buf[0];  // default data buffer
};
typedef struct sf_str* sf_str;

struct sf_slist {
  sf_str*   data;   // sf_str list
  unsigned  len;    // used length
  unsigned  _size;  // count of sf_str
};
typedef struct sf_slist* sf_slist;

extern bool sf_str_catb(sf_str dst, const char* src, unsigned len);
extern bool sf_str_copyb(sf_str dst, const char* src, unsigned len);
extern int  sf_str_cmp(sf_str str1, sf_str str2);
extern int  sf_str_consume(sf_str str, unsigned len);
extern int  sf_str_rconsume(sf_str str, unsigned len);
extern int  sf_str_search(sf_str str, const char* key);
extern bool sf_str_split(sf_str str, const char* key, sf_slist slist);
extern void sf_str_lstrim(sf_str str);
extern void sf_str_rstrim(sf_str str);
extern bool sf_str_tolower(sf_str str);
extern bool sf_str_toupper(sf_str str);

extern sf_str sf_str_new(unsigned default_size);
extern bool   sf_str_del(sf_str str);

static inline 
bool sf_str_cat(sf_str dst, sf_str src) {
  if (!dst || !src) return false;
  return sf_str_catb(dst, src->data, src->len);
}

static inline 
bool sf_str_cats(sf_str dst, const char* src) {
  if (!dst || !src) return false;
  return sf_str_catb(dst, src, strlen(src));
}

static inline 
bool sf_str_copy(sf_str dst, sf_str src) {
  if (!dst || !src) return false;
  return sf_str_copyb(dst, src->data, src->len);
}

static inline 
bool sf_str_copys(sf_str dst, const char* src) {
  if (!dst || !src) return false;
  return sf_str_copyb(dst, src, strlen(src));
}

static inline 
void sf_str_clean(sf_str str) {
  if (!str) return;
  str->data = str->_start;
  str->len = 0;
  str->data[str->len] = 0;
}

static inline 
void sf_str_strim(sf_str str) {
  sf_str_lstrim(str);
  sf_str_rstrim(str);
}

extern bool     sf_slist_addb(sf_slist slist, char* str, unsigned len);

static inline
sf_slist sf_slist_new() {
  return calloc(1, sizeof(struct sf_slist));
}

static inline
bool sf_slist_del(sf_slist slist) {
  if (!slist) return false;
  for (int i = 0; i < slist->_size; i++) sf_str_del(slist->data[i]);
  free(slist->data);
  free(slist);
  return true;
}
static inline 
bool sf_slist_add(sf_slist slist, sf_str str) {
  if (!slist || !str) return false;
  return sf_slist_addb(slist, str->data, str->len);
}

static inline 
bool sf_slist_adds(sf_slist slist, char* str) {
  if (!slist || !str) return false;
  return sf_slist_addb(slist, str, strlen(str));
}

static inline 
void sf_slist_clean(sf_slist slist) {
  if (!slist) return;
  slist->len = 0;
}

#endif

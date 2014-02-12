#ifndef __SF_STRING_H
#define __SF_STRING_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct sf_string_s {
  char*     data;
  unsigned  len;      // used length, string length
  unsigned  _size;    // total size
  char*     _start;   // point to the real buffer
  char      _buf[0];  // default data buffer
};
typedef struct sf_string_s sf_string_t;

struct sf_slist_s {
  sf_string_t** data;   // sf_string list
  unsigned      len;    // used length
  unsigned      _size;  // count of sf_string
};
typedef struct sf_slist_s sf_slist_t;

extern bool 
sf_string_catb(sf_string_t* dst, const char* src, unsigned len);

extern bool 
sf_string_copyb(sf_string_t* dst, const char* src, unsigned len);

extern int  
sf_string_cmp(sf_string_t* str1, sf_string_t* str2);

extern int  
sf_string_consume(sf_string_t* str, unsigned len);

extern int  
sf_string_rconsume(sf_string_t* str, unsigned len);

extern int  
sf_string_search(sf_string_t* str, const char* key);

extern sf_slist_t* 
sf_string_split(sf_string_t* str, const char* key);

extern void 
sf_string_lstrim(sf_string_t* str);

extern void 
sf_string_rstrim(sf_string_t* str);

extern bool 
sf_string_tolower(sf_string_t* str);

extern bool 
sf_string_toupper(sf_string_t* str);

extern sf_string_t* 
sf_string_new(unsigned default_size);

extern bool   
sf_string_del(sf_string_t* str);

static inline bool 
sf_string_cat(sf_string_t* dst, sf_string_t* src) {
  if (!dst || !src) return false;
  return sf_string_catb(dst, src->data, src->len);
}

static inline bool 
sf_string_cats(sf_string_t* dst, const char* src) {
  if (!dst || !src) return false;
  return sf_string_catb(dst, src, strlen(src));
}

static inline bool 
sf_string_copy(sf_string_t* dst, sf_string_t* src) {
  if (!dst || !src) return false;
  return sf_string_copyb(dst, src->data, src->len);
}

static inline bool 
sf_string_copys(sf_string_t* dst, const char* src) {
  if (!dst || !src) return false;
  return sf_string_copyb(dst, src, strlen(src));
}

static inline void 
sf_string_clean(sf_string_t* str) {
  if (!str) return;
  str->data = str->_start;
  str->len = 0;
  str->data[str->len] = 0;
}

static inline void 
sf_string_strim(sf_string_t* str) {
  sf_string_lstrim(str);
  sf_string_rstrim(str);
}

extern bool 
sf_slist_appendb(sf_slist_t* slist, char* str, unsigned len);

static inline sf_slist_t*
sf_slist_new() {
  return calloc(1, sizeof(sf_slist_t));
}

static inline bool 
sf_slist_del(sf_slist_t* slist) {
  if (!slist) return false;
  for (int i = 0; i < slist->_size; i++) sf_string_del(slist->data[i]);
  free(slist->data);
  free(slist);
  return true;
}

static inline bool 
sf_slist_append(sf_slist_t* slist, sf_string_t* str) {
  if (!slist || !str) return false;
  return sf_slist_appendb(slist, str->data, str->len);
}

static inline bool 
sf_slist_appends(sf_slist_t* slist, char* str) {
  if (!slist || !str) return false;
  return sf_slist_appendb(slist, str, strlen(str));
}

static inline void 
sf_slist_clean(sf_slist_t* slist) {
  if (!slist) return;
  slist->len = 0;
}

#endif

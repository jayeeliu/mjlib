#include "sf_string.h"

/*
===============================================================================
sf_string_ready
  alloc enough size for sf_string
===============================================================================
*/
static bool 
sf_string_ready(sf_string_t* str, unsigned need_size) {
  // 1. have enough size, return true
  if (need_size <= str->_size - (str->data - str->_start)) return true;
  // 2. no need to realloc
  if (need_size <= str->_size) {
    memmove(str->_start, str->data, str->len);
    str->data           = str->_start;
    str->data[str->len] = 0;
    return true;
  }
  // 3. need to realloc, get new size to alloc
  unsigned new_size = 30 + need_size + (need_size >> 3);
  char* new_data = malloc(new_size);
  if (!new_data) return false;
  memcpy(new_data, str->data, str->len); 
  // free old buffer
  if (str->_start != str->_buf) free(str->_start);
  str->_size          = new_size; 
  str->_start         = new_data;
  str->data           = str->_start;
  str->data[str->len] = 0;
  return true;
}

/*
===============================================================================
sf_string_readyplus
  call sf_string_ready
===============================================================================
*/
static inline bool 
sf_string_readyplus(sf_string_t* str, unsigned need_size_plus) {
  return sf_string_ready(str, str->len + need_size_plus);
}

/*
===============================================================================
sf_string_catb
  cat binary string to sf_string
===============================================================================
*/
bool 
sf_string_catb(sf_string_t* dst, const char* src, unsigned len) {
  if (!dst || !src) return false;
  if (!sf_string_readyplus(dst, len + 1)) return false;
  // copy string
  memcpy(dst->data + dst->len, src, len);            
  dst->len            += len;
  dst->data[dst->len] = 0;                
  return true;
}

/*
===============================================================================
sf_string_copyb
  copy binary string to sf_string
===============================================================================
*/
bool 
sf_string_copyb(sf_string_t* dst, const char* src, unsigned len) {
  if (!dst || !src) return false;
  if (!sf_string_ready(dst, len + 1)) return false;
  memcpy(dst->_start, src, len);
  dst->len            = len;          
  dst->data           = dst->_start;
  dst->data[dst->len] = 0;
  return true;
}

/*
===============================================================================
sf_string_consume
  consume len string from left
===============================================================================
*/
int 
sf_string_consume(sf_string_t* str, unsigned len) {
  if (!str || !len) return 0;
  if (len >= str->len) {
    int ret = str->len;
    sf_string_clean(str);
    return ret; 
  }
  str->data           += len;
  str->len            -= len;
  str->data[str->len] = 0;
  return len;
}

/*
===============================================================================
sf_string_rconsume
  consume str from right
===============================================================================
*/
int 
sf_string_rconsume(sf_string_t* str, unsigned len) {
  if (!str || !len) return 0;
  if (str->len < len) {
    str->data = str->_start;
    str->len  = 0;
  } else {
    str->len  -= len;
  }
  str->data[str->len] = 0;
  return len;
}

/*
===============================================================================
sf_string_search
    search string in x
    return startpositon in x
            -1 for no found or error
===============================================================================
*/
int 
sf_string_search(sf_string_t* str, const char* key) {
  if (!str || !str->len || !key) return -1;
  char* point = strstr(str->data, key);
  if (!point) return -1;
  return point - str->data;
}

/*
===============================================================================
sf_string_lstrim
  strim string from left
===============================================================================
*/
void 
sf_string_lstrim(sf_string_t* str) {
  if (!str) return;
  int pos;
  for (pos = 0; pos < str->len; pos++) {
    if (str->data[pos] == '\t' || str->data[pos] == ' ' ||
        str->data[pos] == '\r' || str->data[pos] == '\n') continue;
    break;
  }
  sf_string_consume(str, pos);
}

/*
===============================================================================
sf_string_rstrim
  strim string from right
===============================================================================
*/
void 
sf_string_rstrim(sf_string_t* str) {
  if (!str) return;
  int pos;
  for (pos = str->len - 1; pos >= 0; pos--) {
    if (str->data[pos] == '\t' || str->data[pos] == ' ' ||
        str->data[pos] == '\r' || str->data[pos] == '\n') continue;
    break;
  }
  str->len            = pos + 1;
  str->data[str->len] = 0;
}

/*
===============================================================================
sf_string_split
    split sf_string into sf_slist_t
===============================================================================
*/
sf_slist_t*
sf_string_split(sf_string_t* str, const char* key) {
  if (!str || !key) return NULL;

  sf_slist_t* slist = sf_slist_new();
  if (!slist) return NULL;
  // split from left to right
  int start = 0;
  while (start < str->len) {
    // split one by one
    char* point = strstr(str->data + start, key);
    if (!point) break;
    // add to string ignore null
    if (point != str->data + start) {
      sf_slist_appendb(slist, str->data + start, point - str->data - start);
    }
    start = point - str->data + strlen(key);
  }
  if (str->len != start) {
    sf_slist_appendb(slist, str->data + start, str->len - start);  
  }
  return slist;
}

/*
===============================================================================
sf_string_cmp
  compare two sf_string
===============================================================================
*/
int 
sf_string_cmp(sf_string_t* str1, sf_string_t* str2) {
  if (!str1 && !str2) return 0;
  if (!str1 && str2) return -1;
  if (str1 && !str2) return 1;
  // get minlen
  int minlen = (str1->len > str2->len) ? str2->len : str1->len;
  int ret = memcmp(str1->data, str2->data, minlen);
  if (ret) return ret;
  // length is equal  
  if (str1->len == str2->len) return 0;
  if (str1->len > str2->len) return 1;
  return -1;
}

/*
===============================================================================
sf_string_tolower
  change sf_string to lower
===============================================================================
*/
bool 
sf_string_tolower(sf_string_t* str) {
  if (!str) return false;
  for (int i=0; i<str->len; i++) {
    if (str->data[i]>='A' && str->data[i]<='Z') str->data[i] += 32;
  }
  return true;
}

/*
===============================================================================
sf_string_toupper
  change sf_string to capitable
===============================================================================
*/
bool 
sf_string_toupper(sf_string_t* str) {
  if (!str) return false;
  for (int i=0; i <str->len; i++) {
    if (str->data[i]>='a' && str->data[i]<='z') str->data[i] -= 32;
  }
  return true;
}

/*
===============================================================================
sf_string_new
    create new sf_string
===============================================================================
*/
sf_string_t* 
sf_string_new(unsigned int default_size) {
  sf_string_t* str = calloc(1, sizeof(sf_string_t)+default_size*sizeof(char));
  if (!str) return NULL;
  str->_start = str->_buf;
  str->data   = str->_start;
  str->_size  = default_size;
  return str;
}

/*
===============================================================================
sf_string_del
    free sf_string
===============================================================================
*/
bool 
sf_string_del(sf_string_t* str) {
  if (!str) return false;
  if (str->_start != str->_buf) free(str->_start);
  free(str);
  return true;
}

/*
===============================================================================
sf_slist_ready
  sf_slist ready
===============================================================================
*/
static bool 
sf_slist_ready(sf_slist_t* slist, unsigned need_size) {
  // have enough space
  unsigned size = slist->_size;
  if (need_size <= size) return true;
  // realloc space
  slist->_size = 30 + need_size + (need_size >> 3);
  sf_string_t** new_data;
  new_data = realloc(slist->data, slist->_size*sizeof(sf_string_t*));
  if (!new_data) {
    slist->_size = size;
    return false;
  }
  slist->data = new_data;
  // clean other
  for (int i = slist->len; i < slist->_size; i++) slist->data[i] = NULL;
  return true;
}

/*
===============================================================================
sf_slist_readyplus
  call sf_slist
===============================================================================
*/
static inline bool 
sf_slist_readyplus(sf_slist_t* slist, unsigned n) {
  return sf_slist_ready(slist, slist->len + n);
}
 
/*
===============================================================================
sf_slist_appendb
  add new string in strList
===============================================================================
*/
bool 
sf_slist_appendb(sf_slist_t* slist, char* str, unsigned len) {
  if (!slist || !str) return false;
  if (!sf_slist_readyplus(slist, 1)) return false;
  // copy string
  if (!slist->data[slist->len]) {
    slist->data[slist->len] = sf_string_new(80);
    if (!slist->data[slist->len]) return false;
  }
  sf_string_copyb(slist->data[slist->len], str, len);
  slist->len++;
  return true;
}

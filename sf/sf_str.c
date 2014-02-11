#include "sf_str.h"

/*
===============================================================================
sf_str_ready
  alloc enough size for sf_str
===============================================================================
*/
static 
bool sf_str_ready(sf_str str, unsigned need_size) {
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
sf_str_readyplus
  call sf_str_ready
===============================================================================
*/
static inline 
bool sf_str_readyplus(sf_str str, unsigned need_size_plus) {
  return sf_str_ready(str, str->len + need_size_plus);
}

/*
===============================================================================
sf_str_catb
  cat binary string to sf_str
===============================================================================
*/
bool sf_str_catb(sf_str dst, const char* src, unsigned len) {
  if (!dst || !src) return false;
  if (!sf_str_readyplus(dst, len + 1)) return false;
  // copy string
  memcpy(dst->data + dst->len, src, len);            
  dst->len            += len;
  dst->data[dst->len] = 0;                
  return true;
}

/*
===============================================================================
sf_str_copyb
  copy binary string to sf_str
===============================================================================
*/
bool sf_str_copyb(sf_str dst, const char* src, unsigned len) {
  if (!dst || !src) return false;
  if (!sf_str_ready(dst, len + 1)) return false;
  memcpy(dst->_start, src, len);
  dst->len            = len;          
  dst->data           = dst->_start;
  dst->data[dst->len] = 0;
  return true;
}

/*
===============================================================================
sf_str_consume
  consume len string from left
===============================================================================
*/
int sf_str_consume(sf_str str, unsigned len) {
  if (!str || !len) return 0;
  if (len >= str->len) {
    int ret = str->len;
    sf_str_clean(str);
    return ret; 
  }
  str->data           += len;
  str->len            -= len;
  str->data[str->len] = 0;
  return len;
}

/*
===============================================================================
sf_str_rconsume
  consume str from right
===============================================================================
*/
int sf_str_rconsume(sf_str str, unsigned len) {
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
sf_str_search
    search string in x
    return startpositon in x
            -1 for no found or error
===============================================================================
*/
int sf_str_search(sf_str str, const char* key) {
  if (!str || !str->len || !key) return -1;
  char* point = strstr(str->data, key);
  if (!point) return -1;
  return point - str->data;
}

/*
===============================================================================
sf_str_lstrim
  strim string from left
===============================================================================
*/
void sf_str_lstrim(sf_str str) {
  if (!str) return;
  int pos;
  for (pos = 0; pos < str->len; pos++) {
    if (str->data[pos] == '\t' || str->data[pos] == ' ' ||
        str->data[pos] == '\r' || str->data[pos] == '\n') continue;
    break;
  }
  sf_str_consume(str, pos);
}

/*
===============================================================================
sf_str_rstrim
  strim string from right
===============================================================================
*/
void sf_str_rstrim(sf_str str) {
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
sf_str_split
    split sf_str into sf_slist
    return: true --- success; false --- failed;
===============================================================================
*/
bool sf_str_split(sf_str str, const char* key, sf_slist slist) {
  if (!str || !key || !slist) return false;
  sf_slist_clean(slist);
  // split from left to right
  int start = 0;
  while (start < str->len) {
    // split one by one
    char* point = strstr(str->data + start, key);
    if (!point) break;
    // add to string ignore null
    if (point != str->data + start) {
      sf_slist_addb(slist, str->data + start, point - str->data - start);
    }
    start = point - str->data + strlen(key);
  }
  if (str->len != start) {
    sf_slist_addb(slist, str->data + start, str->len - start);  
  }
  return true;
}

/*
===============================================================================
sf_str_cmp
  compare two sf_str
===============================================================================
*/
int sf_str_cmp(sf_str str1, sf_str str2) {
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
sf_str_tolower
  change sf_str to lower
===============================================================================
*/
bool sf_str_tolower(sf_str str) {
  if (!str) return false;
  for (int i = 0; i < str->len; i++) {
    if (str->data[i] >= 'A' && str->data[i] <= 'Z') str->data[i] += 32;
  }
  return true;
}

/*
===============================================================================
sf_str_toupper
  change sf_str to capitable
===============================================================================
*/
bool sf_str_toupper(sf_str str) {
  if (!str) return false;
  for (int i = 0; i < str->len; i++) {
    if (str->data[i] >= 'a' && str->data[i] <= 'z') str->data[i] -= 32;
  }
  return true;
}

/*
===============================================================================
sf_str_New 
    create new sf_str
===============================================================================
*/
sf_str sf_str_new(unsigned int default_size) {
  sf_str str = calloc(1, sizeof(struct sf_str) + default_size * sizeof(char));
  if (!str) return NULL;
  str->_start = str->_buf;
  str->data   = str->_start;
  str->_size  = default_size;
  return str;
}

/*
===============================================================================
sf_str_del
    free sf_str
===============================================================================
*/
bool sf_str_del(sf_str str) {
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
static 
bool sf_slist_ready(sf_slist slist, unsigned need_size) {
  // have enough space
  unsigned size = slist->_size;
  if (need_size <= size) return true;
  // realloc space
  slist->_size = 30 + need_size + (need_size >> 3);
  sf_str* new_data = realloc(slist->data, slist->_size * sizeof(sf_str));
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
static inline 
bool sf_slist_readyplus(sf_slist slist, unsigned n) {
  return sf_slist_ready(slist, slist->len + n);
}
 
/*
===============================================================================
sf_slist_addb
  add new string in strList
===============================================================================
*/
bool sf_slist_addb(sf_slist slist, char* str, unsigned len) {
  if (!slist || !str) return false;
  if (!sf_slist_readyplus(slist, 1)) return false;
  // copy string
  if (!slist->data[slist->len]) {
    slist->data[slist->len] = sf_str_new(80);
    if (!slist->data[slist->len]) return false;
  }
  sf_str_copyb(slist->data[slist->len], str, len);
  slist->len++;
  return true;
}

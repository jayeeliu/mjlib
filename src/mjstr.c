#include <stdlib.h>
#include "mjlog.h"
#include "mjstr.h"

/*
===============================================================================
mjstr_ready
  alloc enough size for mjstr
===============================================================================
*/
static bool mjstr_ready(mjstr str, unsigned int need_size) {
  // 1. have enough size, return true
  if (need_size <= str->_total - (str->data - str->_data_start)) return true;
  // 2. no need to realloc
  if (need_size <= str->_total) {
    memmove(str->_data_start, str->data, str->len);
    str->data = str->_data_start;
    str->data[str->len] = 0;
    return true;
  }
  // 3. need to realloc, get new size to alloc
  unsigned int new_total = 30 + need_size + (need_size >> 3);
  char* new_data = (char*) malloc(new_total);
  if (!new_data) return false;
  memcpy(new_data, str->data, str->len); 
  // free old buffer
  if (str->_data_start != str->_data_buf) free(str->_data_start);
  // set str
  str->_total = new_total; 
  str->_data_start = new_data;
  str->data = str->_data_start;
  str->data[str->len] = 0;
  return true;
}

/*
===============================================================================
mjstr_readyplus
  call mjstr_Ready
===============================================================================
*/
static inline bool mjstr_readyplus(mjstr str, unsigned int need_size_plus) {
  return mjstr_ready(str, str->len + need_size_plus);
}

/*
===============================================================================
mjstr_CatB
  cat binary string to mjstr
===============================================================================
*/
bool mjstr_catb(mjstr str, const char* src, unsigned int len) {
  if (!str || !src) return false;
  // extend if needed
  if (!mjstr_readyplus(str, len + 1)) return false;
  // copy string
  memcpy(str->data + str->len, src, len);            
  str->len += len;
  str->data[str->len] = 0;                
  return true;
}

/*
===============================================================================
mjstr_CopyB
  copy binary string to mjstr
===============================================================================
*/
bool mjstr_copyb(mjstr str, const char* src, unsigned int len) {
  // sanity check
  if (!str || !src) return false;
  // extend if needed
  if (!mjstr_ready(str, len + 1)) return false;
  // copy string
  memcpy(str->_data_start, src, len);
  // set length and data
  str->len = len;          
  str->data   = str->_data_start;
  str->data[str->len] = 0;
  return true;
}

/*
===============================================================================
mjstr_Consume
  consume len string from left
===============================================================================
*/
int mjstr_consume(mjstr str, unsigned int len) {
  // sanity check
  if (!str || len <= 0) return 0;
  // len is too large
  if (len >= str->len) {
    int ret = str->len;
    mjstr_clean(str);
    return ret; 
  }
  // move data and consume
  str->data   += len;
  str->len -= len;
  str->data[str->len] = 0;
  return len;
}

/*
===============================================================================
mjstr_RConsume
  consume str from right
===============================================================================
*/
int mjstr_rconsume(mjstr str, unsigned int len) {
  // sanity check
  if (!str || len <= 0) return 0;
  // adjust length
  if (str->len < len) {
    str->data = str->_data_start;
    str->len = 0;
  } else {
    str->len -= len;
  }
  str->data[str->len] = 0;
  return len;
}

/*
===============================================================================
mjstr_Search
    search string in x
    return startpositon in x
            -1 for no found or error
===============================================================================
*/
int mjstr_search(mjstr str, const char* split) {
  // sanity check
  if (!str || !str->len || !split) return -1;
  // get split point
  char* point = strstr(str->data, split);
  if (!point) return -1;
  return point - str->data;
}

/*
===============================================================================
mjstr_LStrim
  strim string from left
===============================================================================
*/
void mjstr_lstrim(mjstr str) {
  // sanity check
  if (!str) return;
  // get pos 
  int pos;
  for (pos = 0; pos < str->len; pos++) {
    if (str->data[pos] == '\t' || str->data[pos] == ' ' ||
        str->data[pos] == '\r' || str->data[pos] == '\n') continue;
    break;
  }
  mjstr_consume(str, pos);
}

/*
===============================================================================
mjstr_Rstrim
  strim string from right
===============================================================================
*/
void mjstr_rstrim(mjstr str) {
  // sanity check
  if (!str) return;
  // get pos from right
  int pos;
  for (pos = str->len - 1; pos >= 0; pos--) {
    if (str->data[pos] == '\t' || str->data[pos] == ' ' ||
        str->data[pos] == '\r' || str->data[pos] == '\n') continue;
    break;
  }
  str->len = pos + 1;
  str->data[str->len] = 0;
}

/*
===============================================================================
mjstr_Split
    split mjstr into mjslist
    return: true --- success; false --- failed;
===============================================================================
*/
bool mjstr_split(mjstr str, const char* split, mjslist slist) {
  // sanity check
  if (!str || !split || !slist) return false;
  // split from left to right
  int start = 0;
  while (start < str->len) {
    // split one by one
    char* point = strstr(str->data + start, split);
    if (!point) break;
    // add to string
    if (point != str->data + start) {
      mjslist_addb(slist, str->data + start, point - str->data - start);
    }
    start = point - str->data + strlen(split);
  }
  mjslist_addb(slist, str->data + start, str->len - start);  
  return true;
}

/*
===============================================================================
mjstr_Cmp
  compare two mjstr
===============================================================================
*/
int mjstr_cmp(mjstr str1, mjstr str2) {
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
mjstr_ToLower
  change mjstr to lower
===============================================================================
*/
bool mjstr_tolower(mjstr str) {
  // sanity check
  if (!str) return false;
  // change string
  for (int i = 0; i < str->len; i++) {
    if (str->data[i] >= 'A' && str->data[i] <= 'Z') str->data[i] += 32;
  }
  return true;
}

/*
===============================================================================
mjstr_ToUpper
  change mjstr to capitable
===============================================================================
*/
bool mjstr_toupper(mjstr str) {
  // sanity check
  if (!str) return false;
  // change string
  for (int i = 0; i < str->len; i++) {
    if (str->data[i] >= 'a' && str->data[i] <= 'z') str->data[i] -= 32;
  }
  return true;
}

/*
===============================================================================
mjstr_New 
    create new mjstr
===============================================================================
*/
mjstr mjstr_new(unsigned int size) {
  mjstr str = (mjstr) calloc(1, sizeof(struct mjstr) + size * sizeof(char));
  if (!str) return NULL;
  str->_data_start = str->_data_buf;
  str->data = str->_data_start;
  str->_total = size;
  return str;
}

/*
===============================================================================
mjstr_Delete
    free mjstr
===============================================================================
*/
bool mjstr_delete(mjstr str) {
  if (!str) return false;
  if (str->_data_start != str->_data_buf) free(str->_data_start);
  free(str);
  return true;
}

/*
===============================================================================
mjslist_Ready
  mjslist ready
===============================================================================
*/
static bool mjslist_ready(mjslist slist, unsigned int need_size) {
  // have enough space
  unsigned int total = slist->_total;
  if (need_size <= total) return true;
  // realloc space
  slist->_total = 30 + need_size + (need_size >> 3);
  mjstr* new_data = (mjstr*) realloc(slist->data, slist->_total*sizeof(mjstr));
  if (!new_data) {
    slist->_total = total;
    return false;
  }
  slist->data = new_data;
  // clean other
  for (int i = slist->len; i < slist->_total; i++) slist->data[i] = NULL;
  return true;
}

/*
===============================================================================
mjslist_ReadyPlus
  call mjslist
===============================================================================
*/
static inline bool mjslist_readyplus(mjslist slist, unsigned int n) {
  return mjslist_ready(slist, slist->len + n);
}
 
/*
===============================================================================
mjslist_AddB
  add new string in strList
===============================================================================
*/
bool mjslist_addb(mjslist slist, char* str, int len) {
  // sanity check
  if (!slist || !str) return false;
  // alloc enough space
  if (!mjslist_readyplus(slist, 1)) return false;
  // copy string
  if (!slist->data[slist->len]) {
    slist->data[slist->len] = mjstr_new(80);
    if (!slist->data[slist->len]) return false;
  }
  mjstr_copyb(slist->data[slist->len], str, len);
  slist->len++;
  return true;
}

/*
===============================================================================
mjslist_New
    alloc new mjslist struct
===============================================================================
*/
mjslist mjslist_new() {
  mjslist slist = (mjslist) calloc(1, sizeof(struct mjslist));
  if (!slist) return NULL;
  return slist;
}

/*
===============================================================================
mjslist_Delete
    delete mjslist
===============================================================================
*/
bool mjslist_delete(mjslist slist) {
  if (!slist) return false;
  // clean strlist
  for (int i = 0; i < slist->_total; i++) {
    if (slist->data[i]) mjstr_delete(slist->data[i]);
  }
  free(slist->data);
  free(slist);
  return true;
}

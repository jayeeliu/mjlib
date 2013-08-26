#include <stdlib.h>
#include <string.h>
#include "mjlog.h"
#include "mjstr2.h"

/*
===============================================================================
mjstr_Ready
  alloc enough size for mjstring
===============================================================================
*/
static bool mjstr_ready(mjstr str, unsigned int need_size) {
  // string must not be null
  if (!str) return false;
  // 1. have enough size, return true
  if (need_size <= str->_total - str->_start) return true;
  // 2. no need to realloc
  if (need_size <= str->_total) {
    memmove(str->_data, str->_data + str->_start, str->_length);
    str->_start = 0;
    return true;
  }
  // 3. need to realloc, get new size to alloc
  unsigned int new_total = 30 + need_size + (need_size >> 3);
  char* new_data = (char*) malloc(new_total);
  if (!new_data) return false;
  memcpy(new_data, str->_data + str->_start, str->_length); 
  // free old buffer
  if (str->_data != str->_data_buf) free(str->_data);
  // set str
  str->_start = 0;
  str->_total = new_total; 
  str->_data  = new_data;
  str->_data[str->_start + str->_length] = 0;
  return true;
}

/*
===============================================================================
mjstr_ReadyPlus
  call mjstr_Ready
===============================================================================
*/
static bool mjstr_readyplus(mjstr str, unsigned int need_size_plus) {
  return mjstr_ready(str, str->_length + need_size_plus);
}

/*
===============================================================================
mjstr_Copy
  copy mjstr call mjstr_copyb
===============================================================================
*/
bool mjstr_copy(mjstr str_to, mjstr str_from) {
  if (!str_to || !str_from) return false;
  return mjstr_copyb(str_to, str_from->_data + str_from->_start, 
      str_from->_length);
}

/*
===============================================================================
mjstr_CopyS
  copy string to mjstr call mjstr_copyb
===============================================================================
*/
bool mjstr_copys(mjstr str, const char* src) {
  if (!str || !src) return false;
  return mjstr_copyb(str, src, strlen(src));
}

/*
===============================================================================
mjstr_CopyB
  copy binary string to mjstr
===============================================================================
*/
bool mjstr_copyb(mjstr str, const char* src, unsigned int len) {
  // extend if needed
  if (!mjstr_ready(str, len + 1)) return false;
  // copy string
  memcpy(str->_data, src, len);
  // set length and data
  str->_start   = 0;
  str->_length  = len;          
  str->_data[str->_start + str->_length] = 0;
  return true;
}

/*
===============================================================================
mjstr_Cat
  cat mjstr call mjstr_catb
===============================================================================
*/
bool mjstr_cat(mjstr str_to, mjstr str_from) {
  return mjstr_catb(str_to, str_from->_data + str_from->_start, 
      str_from->_length);
}

/*
===============================================================================
mjstr_CatS
  cat string to mjstr call mjstr_catb
===============================================================================
*/
bool mjstr_cats(mjstr str, const char* src) {
  return mjstr_catb(str, src, strlen(src));
}

/*
===============================================================================
mjstr_CatB
  cat binary string to mjstr
===============================================================================
*/
bool mjstr_catb(mjstr str, const char* src, unsigned int len) {
  // extend if needed
  if (!mjstr_readyplus(str, len + 1)) return false;
  // copy string
  memcpy(str->_data + str->_start + str->_length, src, len);            
  str->_length += len;
  str->_data[str->_start + str->_length] = 0;                
  return true;
}

/*
===============================================================================
mjstr_clean
  clean mjstr to null string
===============================================================================
*/
bool mjstr_clean(mjstr str) {
  if (!str) return false;
  str->_start = 0;
  str->_length = 0;
  str->_data[0] = 0;
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
  if (len <= 0) return 0;
  // len is too large
  if (len >= str->_length) {
    int ret = str->_length;
    mjstr_clean(str);
    return ret; 
  }
  // move data and consume
  str->_start   += len;
  str->_length  -= len;
  str->_data[str->_start + str->_length] = 0;
  return len;
}

/*
===============================================================================
mjstr_tostr
  return mjstr char*
===============================================================================
*/
char* mjstr_tochar(mjstr str) {
  return str->_data + str->_start;
}

/*
===============================================================================
mjstr_get_length
  get mjstr length
===============================================================================
*/
int mjstr_get_length(mjstr str) {
  if (!str) return -1;
  return str->_length;
}

/*
===============================================================================
mjstr_RConsume
  consume str from right
===============================================================================
*/
int mjstr_rconsume(mjstr str, unsigned int len) {
  // sanity check
  if (len <= 0) return 0;
  // adjust length
  if (str->_length < len) {
    str->_start = 0;
    str->_length = 0;
  } else {
    str->_length -= len;
  }
  str->_data[str->_start + str->_length] = 0;
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
  if (str == NULL || str->_data == NULL || str->_length == 0 || split == NULL)
    return -1;
  // get split point
  char* point = strstr(str->_data + str->_start, split);
  if (point == NULL) return -1;
  return point - str->_data - str->_start;
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
  for (pos = str->_start; pos < str->_start + str->_length; pos++) {
    if (str->_data[pos] == '\t' || str->_data[pos] == ' ' ||
        str->_data[pos] == '\r' || str->_data[pos] == '\n') continue;
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
  for (pos = str->_start + str->_length - 1; pos >= str->_start; pos--) {
    if (str->_data[pos] == '\t' || str->_data[pos] == ' ' ||
        str->_data[pos] == '\r' || str->_data[pos] == '\n') continue;
    break;
  }
  str->_length = pos + 1 - str->_start;
  str->_data[str->_start + str->_length] = 0;
}

/*
===============================================================================
mjstr_Strim
  strim left and right
===============================================================================
*/
void mjstr_strim(mjstr x) {
  mjstr_lstrim(x);
  mjstr_rstrim(x);
}

/*
===============================================================================
mjstr_Split
    split mjstr into mjstrlist
    return: true --- success; false --- failed;
===============================================================================
*/
bool mjstr_split(mjstr str, const char* split, mjstrlist str_list) {
  // sanity check
  if (!str || !str_list) return false;
  // split from left to right
  int start = 0;
  while (start < str->_length) {
    // split one by one
    char* point = strstr(str->_data + str->_start + start, split);
    if (!point) break;
    // add to string
    if (point - str->_data - str->_start != start) {
      mjstrlist_addb(str_list, str->_data + str->_start + start, 
          point - str->_data - str->_start - start);
    }
    start = point - str->_data - str->_start + strlen(split);
  }
  mjstrlist_addb(str_list, str->_data + str->_start + start, 
      str->_length - start);  
  return true;
}

/*
===============================================================================
mjstr_Cmp
  compare two mjstr
===============================================================================
*/
int mjstr_cmp(mjstr str1, mjstr str2) {
  if (str1 == NULL && str2 == NULL) return 0;
  if (str1 == NULL && str2 != NULL) return -1;
  if (str1 != NULL && str2 == NULL) return 1;
  // get minlen
  int minlen = (str1->_length > str2->_length) ? str2->_length : str1->_length;
  int ret = memcmp(str1->_data + str1->_start, str2->_data + str2->_start, 
      minlen);
  if (ret != 0) return ret;
  // length is equal  
  if (str1->_length == str2->_length) return 0;
  if (str1->_length > str2->_length) return 1;
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
  if (!str) {
    MJLOG_ERR("str is null");
    return false;
  }
  // change string
  for (int i = str->_start; i < str->_start + str->_length; i++) {
    if (str->_data[i] >= 'A' && str->_data[i] <= 'Z') str->_data[i] += 32;
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
  if (!str) {
    MJLOG_ERR( "str is Null" );
    return false;
  }
  // change string
  for (int i = str->_start; i < str->_start + str->_length; i++) {
    if (str->_data[i] >= 'a' && str->_data[i] <= 'z') str->_data[i] -= 32;
  }
  return true;
}

/*
===============================================================================
mjstr_New 
    create new mjstr
===============================================================================
*/
mjstr mjstr_new(unsigned int default_len) {
  mjstr str = (mjstr) calloc(1, sizeof(struct mjstr) + 
      default_len * sizeof(char));
  if (!str) return NULL;
  str->_data = str->_data_buf;
  str->_total = default_len;
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
  if (str->_data != str->_data_buf) free(str->_data);
  free(str);
  return true;
}

/*
===============================================================================
mjstrlist_Ready
  mjstrlist ready
===============================================================================
*/
static bool mjstrlist_ready(mjstrlist str_list, unsigned int need_size) {
  // sanity check
  if (!str_list) return false;
  // have enough space
  unsigned int total = str_list->_total;
  if (need_size <= total) return true;
  // realloc space
  str_list->_total = 30 + need_size + (need_size >> 3);
  mjstr* new_data = (mjstr*) realloc(str_list->_data, 
      str_list->_total * sizeof(mjstr));
  if (!new_data) {
    str_list->_total = total;
    return false;
  }
  str_list->_data = new_data;
  // clean other
  for (int i = str_list->_length; i < str_list->_total; i++) 
    str_list->_data[i] = NULL;
  return true;
}

/*
===============================================================================
mjstrlist_ReadyPlus
  call mjstrlist
===============================================================================
*/
static bool mjstrlist_readyplus(mjstrlist str_list, unsigned int n) {
  return mjstrlist_ready(str_list, str_list->_length + n);
}
 
/*
===============================================================================
mjstrlist_Add
  add mjstr to strList
===============================================================================
*/
bool mjstrlist_add(mjstrlist str_list, mjstr str) {
  return mjstrlist_addb(str_list, str->_data + str->_start, str->_length);
}

/*
===============================================================================
mjstrlist_AddS
  add str to strList
===============================================================================
*/
bool mjstrlist_adds(mjstrlist strList, char* str) {
  return mjstrlist_addb(strList, str, strlen(str));
}

/*
===============================================================================
mjstrlist_AddB
  add new string in strList
===============================================================================
*/
bool mjstrlist_addb(mjstrlist str_list, char* str, int len) {
  // sanity check
  if (!str_list) return false;
  // alloc enough space
  if (!mjstrlist_readyplus(str_list, 1)) return false;
  // copy string
  if (!str_list->_data[str_list->_length]) {
    str_list->_data[str_list->_length] = mjstr_new(80);
    if (!str_list->_data[str_list->_length]) return false;
  }
  mjstr_copyb(str_list->_data[str_list->_length], str, len);
  str_list->_length++;
  return true;
}

/*
===============================================================================
mjstrlist_Get
    get idx in strList, idx range from 0 to length -1
===============================================================================
*/
mjstr mjstrlist_get(mjstrlist str_list, unsigned int idx) {
  if (!str_list || idx >= str_list->_length) return NULL;
  return str_list->_data[idx];
}

/*
===============================================================================
mjstrlist_Clean
    clean mjstrlist, length set to zero
===============================================================================
*/
bool mjstrlist_clean(mjstrlist str_list) {
  if (!str_list) return false;
  str_list->_length = 0;    
  return true;
}

/*
===============================================================================
mjstrlist_New
    alloc new mjstrlist struct
===============================================================================
*/
mjstrlist mjstrlist_new() {
  mjstrlist str_list = (mjstrlist) calloc(1, sizeof(struct mjstrlist));
  if (!str_list) return NULL;
  return str_list;
}

/*
===============================================================================
mjstrlist_Delete
    delete mjstrlist
===============================================================================
*/
bool mjstrlist_delete(mjstrlist str_list) {
  if (!str_list) return false;
  // clean strlist
  for (int i = 0; i < str_list->_total; i++) {
    if (str_list->_data[i]) mjstr_delete(str_list->_data[i]);
  }
  free(str_list->_data);
  free(str_list);
  return true;
}

#include <stdlib.h>
#include <string.h>
#include "mjlog.h"
#include "mjstr.h"

/*
===============================================================================
mjstr_Ready
  alloc enough size for mjstring
===============================================================================
*/
static bool mjstr_ready(mjstr x, unsigned int n) {
  // string must not be null
  if (!x) return false;
  // have enough size, return
  unsigned int i = x->total; 
  if (n <= i) return true;
  //  need to realloc, get new size to alloc
  x->total = 30 + n + (n >> 3);
  // realloc new size
  char *tmpstr = (char*) realloc(x->data, x->total);
  if (!tmpstr) {
    x->total = i;
    return false;
  }
  // get new memory
  x->data = tmpstr;
  x->data[x->length] = 0;
  return true;
}

/*
===============================================================================
mjstr_ReadyPlus
  call mjstr_Ready
===============================================================================
*/
static bool mjstr_readyplus(mjstr x, unsigned int n) {
  return mjstr_ready(x, x->length + n);
}

/*
===============================================================================
mjstr_CopyB
  copy binary string to mjstr
===============================================================================
*/
bool mjstr_copyb(mjstr sa, const char* s, unsigned int n) {
  // extend if needed
  if (!mjstr_ready(sa, n + 1)) return false;
  // copy string
  memcpy( sa->data, s, n );
  // set length and data
  sa->length  = n;          
  sa->data[n]  = '\0';
  return true;
}

/*
===============================================================================
mjstr_Copy
  copy mjstr call mjstr_copyb
===============================================================================
*/
bool mjstr_copy(mjstr sato, mjstr safrom) {
  if (!sato || !safrom) return false;
  return mjstr_copyb(sato, safrom->data, safrom->length);
}

/*
===============================================================================
mjstr_CopyS
  copy string to mjstr call mjstr_copyb
===============================================================================
*/
bool mjstr_copys(mjstr sa, const char* s) {
  if (!sa || !s) return false;
  return mjstr_copyb(sa, s, strlen(s));
}

/*
===============================================================================
mjstr_CatB
  cat binary string to mjstr
===============================================================================
*/
bool mjstr_catb(mjstr sa, const char *s, unsigned int n) {
  // sa is null copy from s
  if (!sa->data) return mjstr_copyb(sa, s, n);
  // extend if needed
  if (!mjstr_readyplus(sa, n + 1)) return false;
  // copy string
  memcpy(sa->data + sa->length, s, n);            
  sa->length += n;
  sa->data[sa->length] = '\0';                
  return true;
}

/*
===============================================================================
mjstr_Cat
  cat mjstr call mjstr_catb
===============================================================================
*/
bool mjstr_cat(mjstr sato, mjstr safrom) {
  return mjstr_catb(sato, safrom->data, safrom->length);
}

/*
===============================================================================
mjstr_CatS
  cat string to mjstr call mjstr_catb
===============================================================================
*/
bool mjstr_cats(mjstr sa, const char *s) {
  return mjstr_catb(sa, s, strlen(s));
}

/*
===============================================================================
mjstr_Consume
  consume len string from left
===============================================================================
*/
int mjstr_consume(mjstr x, unsigned int len) {
  // sanity check
  if (len <= 0) return 0;
  // len is too large
  if (len >= x->length) {
    int ret = x->length;
    x->length = 0;
    x->data[x->length] = 0;
    return ret; 
  }
  // move data and consume
  memmove(x->data, x->data + len, x->length - len);
  x->length -= len;
  x->data[x->length] = 0;
  return len;
}

/*
===============================================================================
mjstr_RConsume
  consume str from right
===============================================================================
*/
int mjstr_rconsume(mjstr x, unsigned int len) {
  // sanity check
  if (len <= 0) return 0;
  // adjust length
  if (x->length < len) {
    x->length = 0;
  } else {
    x->length -= len;
  }
  x->data[x->length] = 0;
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
int mjstr_search(mjstr x, const char* split) {
  // sanity check
  if (x == NULL || x->data == NULL || split == NULL) return -1;
  if (x->length == 0) return -1;
  // get split point
  char *point = strstr(x->data, split);
  if (point == NULL) return -1;
  return point - x->data;
}

/*
===============================================================================
mjstr_LStrim
  strim string from left
===============================================================================
*/
void mjstr_lstrim(mjstr x) {
  // sanity check
  if (!x) return;
  // get pos 
  int pos;
  for (pos = 0; pos < x->length; pos++) {
    if (x->data[pos] == '\t' || x->data[pos] == ' ' ||
        x->data[pos] == '\r' || x->data[pos] == '\n') continue;
    break;
  }
  mjstr_consume(x, pos);
}

/*
===============================================================================
mjstr_Rstrim
  strim string from right
===============================================================================
*/
void mjstr_rstrim(mjstr x) {
  // sanity check
  if (!x) return;
  // get pos from right
  int pos;
  for (pos = x->length - 1; pos >= 0; pos--) {
    if (x->data[pos] == '\t' || x->data[pos] == ' ' ||
        x->data[pos] == '\r' || x->data[pos] == '\n') continue;
    break;
  }
  x->length = pos + 1;
  x->data[x->length] = 0;
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
bool mjstr_split(mjstr x, const char* split, mjstrlist strList) {
  // sanity check
  if (!x || !strList) return false;
  // split from left to right
  int start = 0;
  while (start < x->length) {
    // split one by one
    char* point = strstr(x->data + start, split);
    if (!point) break;
    // add to string
    if (point - x->data != start) {
      mjstrlist_addb(strList, x->data + start, point - x->data - start);
    }
    start = point - x->data + strlen(split);
  }
  mjstrlist_addb(strList, x->data + start, x->length - start);  
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
  int minlen = (str1->length > str2->length) ? str2->length : str1->length;
  int ret = memcmp(str1->data, str2->data, minlen);
  if (ret != 0) return ret;
  // length is equal  
  if (str1->length == str2->length) return 0;
  if (str1->length > str2->length) return 1;
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
  for (int i = 0; i < str->length; i++) {
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
  if (!str) {
    MJLOG_ERR( "str is Null" );
    return false;
  }
  // change string
  for (int i = 0; i < str->length; i++) {
    if (str->data[i] >= 'a' && str->data[i] <= 'z') str->data[i] -= 32;
  }
  return true;
}

/*
===============================================================================
mjstr_Init
  init mjstr
===============================================================================
*/
bool mjstr_init(mjstr str) {
  if (!str) return false;
  str->data   = NULL;
  str->length = 0;
  str->total  = 0;
  return true;
}

/*
===============================================================================
mjstr_DeInit
    deinit mjstr
===============================================================================
*/
bool mjstr_deinit(mjstr str) {
  if (!str) return false;
  free( str->data );
  return true;
}

/*
===============================================================================
mjstr_New 
    create new mjstr
===============================================================================
*/
mjstr mjstr_new() {
  mjstr str = (mjstr) calloc(1, sizeof(struct mjstr));
  if (!str) return NULL;
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
  free(str->data);
  free(str);
  return true;
}

/*
===============================================================================
mjstrlist_Ready
  mjstrlist ready
===============================================================================
*/
static bool mjstrlist_ready(mjstrlist strList, unsigned int n) {
  // sanity check
  if (!strList) return false;
  // have enough space
  unsigned int i = strList->total;
  if (n <= i) return true;
  // realloc space
  strList->total = 30 + n + (n >> 3);
  mjstr* tmp = (mjstr*) realloc(strList->data, strList->total * sizeof(mjstr));
  if (!tmp) {
    MJLOG_ERR("realloc error");
    strList->total = i;
    return false;
  }
  strList->data = tmp;
  // clean other
  for (int i = strList->length; i < strList->total; i++) strList->data[i] = 0;
  return true;
}

/*
===============================================================================
mjstrlist_ReadyPlus
  call mjstrlist
===============================================================================
*/
static bool mjstrlist_readyplus(mjstrlist strList, unsigned int n) {
    return mjstrlist_ready(strList, strList->length + n);
}
 
/*
===============================================================================
mjstrlist_Add
  add mjstr to strList
===============================================================================
*/
bool mjstrlist_add(mjstrlist strList, mjstr str) {
  return mjstrlist_addb(strList, str->data, str->length);
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
bool mjstrlist_addb(mjstrlist strList, char* str, int len) {
  // sanity check
  if (!strList) {
    MJLOG_ERR( "sanity check error" );
    return false;
  }
  // alloc enough space
  if (!mjstrlist_readyplus(strList, 1)) {
    MJLOG_ERR("mjstrlist Ready Error");
    return false;
  }
  // copy string
  if (!strList->data[strList->length]) {
    mjstr tmpStr = mjstr_new();
    if (!tmpStr) {
        MJLOG_ERR( "mjstr_New error" );
        return false;
    }
    strList->data[strList->length] = tmpStr;
  }
  mjstr_copyb(strList->data[strList->length], str, len);
  strList->length++;
  return true;
}

/*
===============================================================================
mjstrlist_Get
    get idx in strList
===============================================================================
*/
mjstr mjstrlist_get( mjstrlist strList, unsigned int idx) {
  if (!strList || idx >= strList->length) {
    MJLOG_ERR("sanity check error");
    return NULL;
  }
  return strList->data[idx];
}

/*
===============================================================================
mjstrlist_Clean
    clean mjstrlist, length set to zero
===============================================================================
*/
bool mjstrlist_clean(mjstrlist strList) {
  if (!strList) {
    MJLOG_ERR( "sanity check error" );
    return false;
  }
  strList->length = 0;    
  return true;
}

/*
===============================================================================
mjstrlist_New
    alloc new mjstrlist struct
===============================================================================
*/
mjstrlist mjstrlist_new() {
  mjstrlist strList = (mjstrlist) calloc(1, sizeof(struct mjstrlist));
  if (!strList) {
    MJLOG_ERR("mjstrlist create error");
    return NULL;
  }
  return strList;
}

/*
===============================================================================
mjstrlist_Delete
    delete mjstrlist
===============================================================================
*/
bool mjstrlist_delete(mjstrlist strList) {
  if (!strList) return false;
  // clean strlist
  for (int i = 0; i < strList->total; i++) {
    if (strList->data[i]) mjstr_delete(strList->data[i]);
  }
  free(strList->data);
  free(strList);
  return true;
}

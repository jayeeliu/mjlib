#include <stdlib.h>
#include <string.h>
#include "mjlog.h"
#include "mjstr.h"

/*
===============================================================================
mjStr_Ready
  alloc enough size for mjString
===============================================================================
*/
static bool mjStr_Ready(mjStr x, unsigned int n) {
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
mjStr_ReadyPlus
  call mjStr_Ready
===============================================================================
*/
static bool mjStr_ReadyPlus(mjStr x, unsigned int n) {
  return mjStr_Ready(x, x->length + n);
}

/*
===============================================================================
mjStr_CopyB
  copy binary string to mjstr
===============================================================================
*/
bool mjStr_CopyB(mjStr sa, const char* s, unsigned int n) {
  // extend if needed
  if (!mjStr_Ready(sa, n + 1)) return false;
  // copy string
  memcpy( sa->data, s, n );
  // set length and data
  sa->length  = n;          
  sa->data[n]  = '\0';
  return true;
}

/*
===============================================================================
mjStr_Copy
  copy mjstr call mjstr_copyb
===============================================================================
*/
bool mjStr_Copy(mjStr sato, mjStr safrom) {
  if (!sato || !safrom) return false;
  return mjStr_CopyB(sato, safrom->data, safrom->length);
}

/*
===============================================================================
mjStr_CopyS
  copy string to mjstr call mjstr_copyb
===============================================================================
*/
bool mjStr_CopyS(mjStr sa, const char* s) {
  if (!sa || !s) return false;
  return mjStr_CopyB(sa, s, strlen(s));
}

/*
===============================================================================
mjStr_CatB
  cat binary string to mjstr
===============================================================================
*/
bool mjStr_CatB(mjStr sa, const char *s, unsigned int n) {
  // sa is null copy from s
  if (!sa->data) return mjStr_CopyB(sa, s, n);
  // extend if needed
  if (!mjStr_ReadyPlus(sa, n + 1)) return false;
  // copy string
  memcpy(sa->data + sa->length, s, n);            
  sa->length += n;
  sa->data[sa->length] = '\0';                
  return true;
}

/*
===============================================================================
mjStr_Cat
  cat mjstr call mjstr_catb
===============================================================================
*/
bool mjStr_Cat(mjStr sato, mjStr safrom) {
  return mjStr_CatB(sato, safrom->data, safrom->length);
}

/*
===============================================================================
mjStr_CatS
  cat string to mjstr call mjstr_catb
===============================================================================
*/
bool mjStr_CatS(mjStr sa, const char *s) {
  return mjStr_CatB(sa, s, strlen(s));
}

/*
===============================================================================
mjStr_Consume
  consume len string from left
===============================================================================
*/
int mjStr_Consume(mjStr x, unsigned int len) {
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
mjStr_RConsume
  consume str from right
===============================================================================
*/
int mjStr_RConsume(mjStr x, unsigned int len) {
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
mjStr_Search
    search string in x
    return startpositon in x
            -1 for no found or error
===============================================================================
*/
int mjStr_Search(mjStr x, const char* split) {
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
mjStr_LStrim
  strim string from left
===============================================================================
*/
void mjStr_LStrim(mjStr x) {
  // sanity check
  if (!x) return;
  // get pos 
  int pos;
  for (pos = 0; pos < x->length; pos++) {
    if (x->data[pos] == '\t' || x->data[pos] == ' ' ||
        x->data[pos] == '\r' || x->data[pos] == '\n') continue;
    break;
  }
  mjStr_Consume(x, pos);
}

/*
===============================================================================
mjStr_Rstrim
  strim string from right
===============================================================================
*/
void mjStr_RStrim(mjStr x) {
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
mjStr_Strim
  strim left and right
===============================================================================
*/
void mjStr_Strim(mjStr x) {
  mjStr_LStrim(x);
  mjStr_RStrim(x);
}

/*
===============================================================================
mjStr_Split
    split mjStr into mjStrList
    return: true --- success; false --- failed;
===============================================================================
*/
bool mjStr_Split(mjStr x, const char* split, mjStrList strList) {
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
      mjStrList_AddB(strList, x->data + start, point - x->data - start);
    }
    start = point - x->data + strlen(split);
  }
  mjStrList_AddB(strList, x->data + start, x->length - start);  
  return true;
}

/*
===============================================================================
mjStr_Cmp
  compare two mjStr
===============================================================================
*/
int mjStr_Cmp(mjStr str1, mjStr str2) {
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
mjStr_ToLower
  change mjstr to lower
===============================================================================
*/
bool mjStr_ToLower(mjStr str) {
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
mjStr_ToUpper
  change mjstr to capitable
===============================================================================
*/
bool mjStr_ToUpper(mjStr str) {
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
mjStr_Init
  init mjstr
===============================================================================
*/
bool mjStr_Init(mjStr str) {
  if (!str) return false;
  str->data   = NULL;
  str->length = 0;
  str->total  = 0;
  return true;
}

/*
===============================================================================
mjStr_DeInit
    deinit mjstr
===============================================================================
*/
bool mjStr_DeInit(mjStr str) {
  if (!str) return false;
  free( str->data );
  return true;
}

/*
===============================================================================
mjStr_New 
    create new mjStr
===============================================================================
*/
mjStr mjStr_New() {
  mjStr str = (mjStr) calloc(1, sizeof(struct mjStr));
  if (!str) return NULL;
  return str;
}

/*
===============================================================================
mjStr_Delete
    free mjStr
===============================================================================
*/
bool mjStr_Delete(mjStr str) {
  if (!str) return false;
  free(str->data);
  free(str);
  return true;
}

/*
===============================================================================
mjStrList_Ready
  mjstrlist ready
===============================================================================
*/
static bool mjStrList_Ready(mjStrList strList, unsigned int n) {
  // sanity check
  if ( !strList ) return false;
  // have enough space
  unsigned int i = strList->total;
  if (n <= i) return true;
  // realloc space
  strList->total = 30 + n + (n >> 3);
  mjStr* tmp = (mjStr*) realloc(strList->data, strList->total * sizeof(mjStr));
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
mjStrList_ReadyPlus
  call mjStrList
===============================================================================
*/
static bool mjStrList_ReadyPlus(mjStrList strList, unsigned int n) {
    return mjStrList_Ready(strList, strList->length + n);
}
 
/*
===============================================================================
mjStrList_Add
  add mjStr to strList
===============================================================================
*/
bool mjStrList_Add(mjStrList strList, mjStr str) {
  return mjStrList_AddB(strList, str->data, str->length);
}

/*
===============================================================================
mjStrList_AddS
  add str to strList
===============================================================================
*/
bool mjStrList_AddS(mjStrList strList, char* str) {
  return mjStrList_AddB(strList, str, strlen(str));
}

/*
===============================================================================
mjStrList_AddB
  add new string in strList
===============================================================================
*/
bool mjStrList_AddB(mjStrList strList, char* str, int len) {
  // sanity check
  if (!strList) {
    MJLOG_ERR( "sanity check error" );
    return false;
  }
  // alloc enough space
  if (!mjStrList_ReadyPlus(strList, 1)) {
    MJLOG_ERR("mjStrList Ready Error");
    return false;
  }
  // copy string
  if (!strList->data[strList->length]) {
    mjStr tmpStr = mjStr_New();
    if (!tmpStr) {
        MJLOG_ERR( "mjStr_New error" );
        return false;
    }
    strList->data[strList->length] = tmpStr;
  }
  mjStr_CopyB(strList->data[strList->length], str, len);
  strList->length++;
  return true;
}

/*
===============================================================================
mjStrList_Get
    get idx in strList
===============================================================================
*/
mjStr mjStrList_Get( mjStrList strList, unsigned int idx) {
  if (!strList || idx >= strList->length) {
    MJLOG_ERR("sanity check error");
    return NULL;
  }
  return strList->data[idx];
}

/*
===============================================================================
mjStrList_Clean
    clean mjStrlist, length set to zero
===============================================================================
*/
bool mjStrList_Clean(mjStrList strList) {
  if (!strList) {
    MJLOG_ERR( "sanity check error" );
    return false;
  }
  strList->length = 0;    
  return true;
}

/*
===============================================================================
mjStrList_New
    alloc new mjStrList struct
===============================================================================
*/
mjStrList mjStrList_New() {
  mjStrList strList = (mjStrList) calloc(1, sizeof(struct mjStrList));
  if (!strList) {
    MJLOG_ERR("mjStrList create error");
    return NULL;
  }
  return strList;
}

/*
===============================================================================
mjStrList_Delete
    delete mjStrList
===============================================================================
*/
bool mjStrList_Delete(mjStrList strList) {
  if (!strList) return false;
  // clean strlist
  for (int i = 0; i < strList->total; i++) {
    if (strList->data[i]) mjStr_Delete(strList->data[i]);
  }
  free(strList->data);
  free(strList);
  return true;
}

#ifndef _MJSTR_H
#define _MJSTR_H

#include <stdbool.h>

// mjStr struct 
struct mjStr {
  unsigned int  length;         // used length 
  unsigned int  total;          // total length
  char*         data;           // point to the string
};
typedef struct mjStr* mjStr;

// mjStrlist struct
struct mjStrList {
  unsigned int  length;         // used length
  unsigned int  total;          // count of mjStr
  mjStr*        data;           // mjStr list
};
typedef struct mjStrList* mjStrList;

// function for mjStr
extern bool   mjStr_Copy(mjStr sato, mjStr safrom);
extern bool   mjStr_CopyS(mjStr sa, const char *s);
extern bool   mjStr_CopyB(mjStr sa, const char *s, unsigned int n);
extern bool   mjStr_Cat(mjStr sato, mjStr safrom);
extern bool   mjStr_CatS(mjStr sa, const char *s);
extern bool   mjStr_CatB(mjStr sa, const char *s, unsigned int n);
extern int    mjStr_Consume(mjStr str, unsigned int len);
extern int    mjStr_RConsume(mjStr str, unsigned int len);
extern int    mjStr_Search( mjStr x, const char *split );
extern int    mjStr_Cmp(mjStr str1, mjStr str2);
extern bool   mjStr_Split(mjStr str, const char* split, mjStrList strList);
extern void   mjStr_Strim(mjStr str);
extern void   mjStr_LStrim(mjStr str);
extern void   mjStr_RStrim(mjStr str);
extern bool   mjStr_ToLower(mjStr str);
extern bool   mjStr_ToUpper(mjStr str);

extern bool   mjStr_Init(mjStr str);
extern bool   mjStr_DeInit(mjStr str);
extern mjStr  mjStr_New();
extern bool   mjStr_Delete(mjStr x);
// function for mjStrList
extern bool       mjStrList_Add(mjStrList strList, mjStr str);
extern bool       mjStrList_AddS(mjStrList strList, char* str);
extern bool       mjStrList_AddB(mjStrList strList, char* str , int len);
extern mjStr      mjStrList_Get(mjStrList strList, unsigned int idx);
extern bool       mjStrList_Clean(mjStrList strList);

extern mjStrList  mjStrList_New();
extern bool       mjStrList_Delete(mjStrList strList);

#endif

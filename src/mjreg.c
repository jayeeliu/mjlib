#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mjlog.h"
#include "mjreg.h"

#define MAXLEN  20

/*
===============================================================================
mjreg_Search
    search string copy result to result
===============================================================================
*/
bool mjreg_search(mjreg reg, char* string, mjStrList result) {
  // call regexec to search string
  regmatch_t pm[MAXLEN];
  if (regexec(&reg->preg, string, MAXLEN, pm, 0)) return false;
  // no result should be return   
  if (!result) return true;
  // copy data 
  for (int i = 0; i < MAXLEN && pm[i].rm_so != -1; i++) {
    mjStrList_AddB(result, string + pm[i].rm_so, pm[i].rm_eo - pm[i].rm_so);
  }
  return true;
}

/*
===============================================================================
mjreg_new
    create new mjreg struct
    return  NULL -- fail,
            other -- success
===============================================================================
*/
mjreg mjreg_new(const char* regex) {
  // create mjreg struct
  mjreg reg = (mjreg) calloc(1, sizeof(struct mjreg));
  if (!reg) {
    MJLOG_ERR("mjreg calloc error");
    return NULL;
  }
  // init reg
  if (regcomp(&reg->preg, regex, REG_EXTENDED)) {
    MJLOG_ERR("regcom error");
    return NULL;
  }
  return reg;
}

/*
===============================================================================
mjreg_delete
    delete mjreg struct
    no return
===============================================================================
*/
bool mjreg_delete(mjreg reg) {
  if (!reg) return false;
  regfree(&reg->preg);
  free(reg);
  return true;
}

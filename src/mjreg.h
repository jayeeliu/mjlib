#ifndef _MJREG_H
#define _MJREG_H

#include <regex.h>
#include "mjstr.h"

struct mjreg {
  regex_t preg;
};
typedef struct mjreg* mjreg;

extern bool   mjreg_search(mjreg reg, char* string, mjStrList result);
extern mjreg  mjreg_new(const char* regex);
extern bool   mjreg_delete(mjreg reg);

#endif

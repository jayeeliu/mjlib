#ifndef _MJREG_H
#define _MJREG_H

#include <regex.h>
#include "mjstr.h"

struct mjReg {
    regex_t preg;
};
typedef struct mjReg* mjReg;

extern bool     mjReg_Search( mjReg reg, char* string, mjStrList result );

extern mjReg    mjReg_New( const char* regex );
extern void     mjReg_Delete( mjReg reg );

#endif

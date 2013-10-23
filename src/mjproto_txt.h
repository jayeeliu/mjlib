#ifndef _MJPROTO_TXT_H
#define _MJPROTO_TXT_H

#include "mjproc.h"
#include "mjstr.h"
#include "mjconnb.h"

struct mjproto_txt_data {
  mjstr   cmd;
  mjslist args;
  mjconnb conn;
  bool    finished;
};
typedef struct mjproto_txt_data* mjproto_txt_data;

struct mjproto_txt_rtlist {
  const char* cmd;
  mjProc      Routine;
};
typedef struct mjproto_txt_rtlist* mjproto_txt_rtlist;

extern void*  mjproto_txt_init(void* arg);
extern void*  mjproto_txt_routine(void* arg);
extern bool   mjproto_txt_finished(mjproto_txt_data cdata);

#endif

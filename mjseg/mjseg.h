#ifndef __MJSEG_H 
#define __MJSEG_H

#include "mjstr.h"
#include "segmentApi.h"
#include <stdbool.h>

struct mjseg {
  PSEGMENTHANDLE            _sgObj;
  PSEGPTHREADRESULTHANDLE   _renode;
  SEG_WORD_SEGMENT_T*       _words;
  int                       _enc;
  int                       _ps;
};
typedef struct mjseg* mjseg;

extern mjslist  mjseg_segment(mjseg seg, char* str);
extern mjseg    mjseg_new(const char* conf_file);
extern bool     mjseg_delete(mjseg seg);

#endif

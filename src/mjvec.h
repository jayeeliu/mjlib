#ifndef __MJVEC_H
#define __MJVEC_H

#include "mjproc.h"
#include <stdbool.h>

struct mjvec {
  unsigned int  _total;
  unsigned int  len;
  mjProc        _obj_free;
  void**        _obj;
};
typedef struct mjvec* mjvec;

extern bool   mjvec_add(mjvec vec, void* value);
extern void*  mjvec_get(mjvec vec, unsigned int idx);
extern mjvec  mjvec_new(mjProc obj_free);
extern bool   mjvec_delete(mjvec vec);

#endif

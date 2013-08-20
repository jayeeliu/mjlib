#ifndef _MJIO_H
#define _MJIO_H

#include "mjstr.h"

struct mjio {
  int 	_fd;
  mjstr	_read_buf;
};
typedef struct mjio* mjio;

extern int  mjio_read(mjio io, mjstr data, int len);
extern int  mjio_readline(mjio io, mjstr data);
extern mjio mjio_new(const char* fileName);
extern bool mjio_delete(mjio io);

#endif

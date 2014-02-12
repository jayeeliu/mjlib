#ifndef __SF_CONN_H
#define __SF_CONN_H

struct sf_conn_s {
  sf_str        buffer;
  unsigned      error:1;
  unsigned      closed:1;
  unsigned      timeout:1;
  sf_object_t*  obj;
};
typedef struct sf_conn_s sf_conn_t;

#endif

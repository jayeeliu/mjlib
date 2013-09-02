#ifndef __MJREDIS_H
#define __MJREDIS_H

#include "hiredis/hiredis.h"
#include "mjstr.h"
#include <stdbool.h>

struct mjredis {
  redisContext* context;
};
typedef struct mjredis* mjredis;

extern int mjredis_get(mjredis redis_handle, const char* key, mjstr out_value);
extern int mjredis_set(mjredis redis_handle, const char* key, const char* value);
extern int mjredis_del(mjredis redis_handle, const char* key);

extern mjredis mjredis_new(const char* ip, int port);
extern bool mjredis_delete(mjredis redis_handle);

#endif

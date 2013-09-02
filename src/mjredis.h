#ifndef __MJREDIS_H
#define __MJREDIS_H

#include "hiredis/hiredis.h"
#include "mjstr.h"
#include <stdbool.h>

#define MAX_IP_LEN 128

struct mjredis {
  char _ip[MAX_IP_LEN];
  int _port;
  redisContext* _context;
};
typedef struct mjredis* mjredis;

extern int mjredis_get(mjredis handle, const char* key, mjstr out_value);
extern int mjredis_set(mjredis handle, const char* key, const char* value);
extern int mjredis_del(mjredis handle, const char* key);
extern int mjredis_lpush(mjredis handle, const char* key, const char* value);
extern int mjredis_rpop(mjredis handle, const char* key, mjstr out_value);

extern mjredis mjredis_new(const char* ip, int port);
extern bool mjredis_delete(mjredis redis_handle);

#endif

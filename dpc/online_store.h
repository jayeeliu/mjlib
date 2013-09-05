#ifndef ONLINE_STORE_H
#define ONLINE_STORE_H 1

#include <stdio.h>
#include "dpc_util.h"
#include "mjconnb.h"
#include "mjthread.h"
#include "mjopt.h"
#include "mjlog.h"
#include "mjproto_txt.h"
#include "mjredis.h"

#define REDIS_EXEC_FAIL_CODE -1


struct online_dsn {
  char host[32];
  int port;
  char database[12];
};


extern void* online_get(void* arg);
extern void* online_put(void* arg);
extern void* online_del(void* arg);
extern void* online_lpush(void* arg);
extern void* online_rpop(void* arg);
extern void* online_llen(void* arg);
extern void* online_quit(void* arg);
extern void* online_create(void* arg);
extern void* online_drop(void* arg);
extern void* online_thread_init(void* arg);


#endif

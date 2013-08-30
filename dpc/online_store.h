#ifndef ONLINE_STORE_H
#define ONLINE_STORE_H 1

#include <stdio.h>
#include "validator.h"
#include "mjconnb.h"
#include "mjproto_txt.h"


extern void* online_get(void* arg);
extern void* online_put(void* arg);
extern void* online_del(void* arg);
extern void* online_quit(void* arg);
extern void* online_create(void* arg);
extern void* online_drop(void* arg);
extern void* online_thread_init(void* arg);


#endif

#include "mjredis.h"

void fun1(mjredis redis_handle) {
  const char* key = "test_mjredis";

  int retval = mjredis_del(redis_handle, key);
  printf("del %d\n", retval);
  
  retval = mjredis_set(redis_handle, key, "mjredis_value11111");
  printf("set %d\n", retval);
 
  mjstr result = mjstr_new(128);
  mjredis_get(redis_handle, key, result);
  if (result) {
    printf("value: %s\n", result->data);
    mjstr_delete(result);
  }
  retval = mjredis_del(redis_handle, key);
  printf("del %d\n", retval);
}

void fun2(mjredis redis_handle) {
  const char* key = "test_mjlist";

  int retval = mjredis_del(redis_handle, key);
  printf("del %d\n", retval);

  retval = mjredis_lpush(redis_handle, key, "value1");
  printf("lpush1 %d\n", retval);
  
  retval = mjredis_lpush(redis_handle, key, "value2");
  printf("lpush2 %d\n", retval);

  mjstr out_value = mjstr_new(128);
  retval = mjredis_rpop(redis_handle, key, out_value);
  printf("retval: %d, rpop1 %s\n", retval, out_value->data);

  retval = mjredis_llen(redis_handle, key);
  printf("llen retval: %d\n", retval);

  retval = mjredis_rpop(redis_handle, key, out_value);
  printf("retval: %d, rpop2 %s\n", retval, out_value->data);

  mjstr_clean(out_value);
  retval = mjredis_rpop(redis_handle, key, out_value);
  printf("retval: %d, rpop3 %s\n", retval, out_value->data);

  retval = mjredis_del(redis_handle, key);
  printf("del %d\n", retval);

  mjstr_delete(out_value);
}

void fun3(mjredis handle) {
  int retval = mjredis_select(handle, "2");
  printf("select 2: %d\n", retval);

  retval = mjredis_select(handle, "1000");
  printf("select 1000: %d\n", retval);
}

void fun4(mjredis handle) {
  char buf[65536] = {0};
  for (int i=0; i<65536; i++) {
    buf[i] = '1';
  }
  for (int i=0; i<20; i++) {
    mjredis_lpush(handle, "test_push", buf);
  }
}

int main() {
  mjredis redis_handle = mjredis_new("127.0.0.1", 6379);
  if (!redis_handle) {
    printf("redis handle error\n");
    return 1;
  }
//  fun1(redis_handle);
//  fun2(redis_handle);
//  fun3(redis_handle);
  fun4(redis_handle);
  mjredis_delete(redis_handle);
  return 0;
}

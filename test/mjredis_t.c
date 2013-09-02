#include "mjredis.h"

int main() {
  mjredis redis_handle = mjredis_new("127.0.0.1", 6379);
  if (!redis_handle) {
    printf("redis handle error\n");
    return 1;
  }
  int retval = mjredis_del(redis_handle, "test_mjredis");
  printf("del %d\n", retval);
  retval = mjredis_set(redis_handle, "test_mjredis", "mjredis_value11111");
  printf("set %d\n", retval);
  mjstr result = mjstr_new(128);
  mjredis_get(redis_handle, "test_mjredis", result);
  if (result) {
    printf("value: %s\n", result->data);
    mjstr_delete(result);
  }
  retval = mjredis_del(redis_handle, "test_mjredis");
  printf("del %d\n", retval);
  mjredis_delete(redis_handle);
  return 0;
}

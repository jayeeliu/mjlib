#include "mjmap.h"
#include <stdio.h>
#include <stdlib.h>

struct mjsched {
};
typedef struct mjsched* mjsched;

struct mjco {
  int     _stage;
  int     _status;    // coroutine status
  mjmap   _map;       // coroutine vars
  mjProc  _func;      // coroutine function
};
typedef struct mjco* mjco;

#define CO_FUNC(name) void name(mjco co) 

#define CO_VAR(type, name) type* name;                              \
   do {                                                             \
    name = mjmap_get_obj(co->_map, #name);                          \
    if (!name) {                                                    \
      name = (type*)calloc(1, sizeof(type));                        \
      mjmap_set_obj(co->_map, #name, name, mjco_var_free);          \
    }                                                               \
  } while(0)

#define CO_BEGIN    switch (co->_stage) { case 0: while(0)
#define CO_RETURN     co->_stage = __LINE__; return; case __LINE__: while(0)
#define CO_END      }; return

void mjco_run(mjco co) {
  if (!co) return;
  co->_func(co);
}

void* mjco_var_free(void* var) {
  free(var);
  return NULL;
}

mjco mjco_new(mjProc func) {
  mjco co = (mjco) calloc(1, sizeof(struct mjco));
  if (!co) return NULL;
  co->_func = func;
  co->_map = mjmap_new(16);
  if (!co->_map) {
    free(co);
    return NULL; 
  }
  return co;
}

bool mjco_delete(mjco co) {
  if (!co) return false;
  if (co->_map) mjmap_delete(co->_map);
  free(co);
  return true;
}

CO_FUNC(foo) {
  CO_VAR(int, a);
  CO_BEGIN;
  *a = 200;
  for (*a = 5; *a < 10;*a = *a + 1) {
    CO_RETURN;
  }
  CO_END;
}

int main() {
  mjco co = mjco_new(foo);
  mjco_run(co);
  mjco_delete(co);
  return 0;
}

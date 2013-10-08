#include "mjmap.h"
#include <stdio.h>
#include <stdlib.h>

struct mjco {
  int     _stage;
  mjmap   _vars;
  mjProc  _func;
};
typedef struct mjco* mjco;

#define CO_FUNC(name) void* name(void* args) 
#define CO_INIT mjco co = (mjco) args

#define CO_VAR(type, name) type* name;                                  \
   do {                                                                 \
    name = mjmap_get_obj(co->_vars, #name);                             \
    if (!name) {                                                        \
      name = (type*)calloc(1, sizeof(type));                            \
      mjmap_set_obj(co->_vars, #name, name, mjco_var_free);             \
    }                                                                   \
  } while(0)

#define CO_BEGIN      switch (co->_stage) { case 0: while(0)
#define CO_RETURN(x)    co->_stage = __LINE__; return x; case __LINE__: while(0)
#define CO_END        }; return NULL

void* mjco_var_free(void* var) {
  free(var);
  return NULL;
}

mjco mjco_new(mjProc func) {
  mjco co = (mjco) calloc(1, sizeof(struct mjco));
  if (!co) return NULL;
  co->_func = func;
  co->_vars = mjmap_new(16);
  if (!co->_vars) {
    free(co);
    return NULL; 
  }
  return co;
}

bool mjco_delete(mjco co) {
  if (!co) return false;
  if (co->_vars) mjmap_delete(co->_vars);
  free(co);
  return true;
}

CO_FUNC(foo) {
  CO_INIT;
  CO_VAR(int, a);
  CO_BEGIN;
  *a = 200;
  for (*a = 5;*a < 10;*a = *a + 1) {
    CO_RETURN(a);
  }
  CO_END;
}

int main() {
  mjco co = mjco_new(foo);
  int* value = foo(co);
  while(value) {
    printf("%d\n", *value);
    value = foo(co);
  }
  mjco_delete(co);
  return 0;
}

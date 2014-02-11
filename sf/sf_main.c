#include "sf_object.h"
#include "sf_module.h"
#include <unistd.h>

int main() {
  sf_modules_init();
  sf_create_worker();
  sf_create_worker();
  sf_modules_start();
  while (1) {
    sleep(3);
  }
  return 0;
}

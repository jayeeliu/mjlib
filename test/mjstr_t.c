#include <stdio.h>
#include "mjstr.h"

int main() {
  mjstr str = mjstr_new(1);
  mjstr_cats(str, "th");
  mjstr_cats(str, "this is the second line this is the second line this is the second line this is the second line this is the second line this is the second line this is the second line this is the second line");
  mjstr_delete(str); 
  return 0;
}

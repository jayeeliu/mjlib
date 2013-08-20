#ifndef __MJOPTION_H
#define __MJOPTION_H

#include <stdbool.h>
#include "mjlist.h"

#define MAX_SECTION_LEN 16
#define MAX_KEY_LEN     16
#define MAX_VALUE_LEN   128

struct mjopt {
	char                section[MAX_SECTION_LEN];
 	char                key[MAX_KEY_LEN];
	char                value[MAX_VALUE_LEN];      // string value
	struct list_head    node;
};
typedef struct mjopt* mjopt;

extern bool mjopt_get_value_int(char* section, char* key, int* retval);
extern bool mjopt_get_value_string(char* section, char* key, char* retval);
extern bool mjopt_parse_conf(const char* fileName);

#endif

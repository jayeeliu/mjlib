#ifndef __MJOPTION_H
#define __MJOPTION_H

#include <stdbool.h>
#include "mjlist.h"

#define MAX_SECTION_LEN 64
#define MAX_KEY_LEN     64
#define MAX_VALUE_LEN   64
#define MAX_CMDKEY_LEN  64

#define MJOPT_NONE      0
#define MJOPT_INT       1
#define MJOPT_STR       2
#define MJOPT_MAX       3

struct mjOpt2 {
    char                section[MAX_SECTION_LEN];
    char                key[MAX_KEY_LEN];
    int                 type;                           // option type
    void*               value;                          // value
    struct list_head    node;
};
typedef struct mjOpt2* mjOpt2;

extern bool mjOpt2_Define( char* section, char* key, int type, void* value, 
                char* defaultValue );
extern bool mjOpt2_ParseConf( const char* fileName );
extern bool mjOpt2_SetValue( char* section, char* key, char* value );

#endif

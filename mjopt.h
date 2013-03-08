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

struct mjOpt {
    char                section[MAX_SECTION_LEN];
    char                key[MAX_KEY_LEN];
    char                cmdKey[MAX_CMDKEY_LEN];         // cmdKey
    int                 cmdKeyValue;                    // if cmdKey has value
    int                 type;                           // option type
    void*               value;                          // value
    char*               helpString;                     // help string 
    struct list_head    node;
};
typedef struct mjOpt* mjOpt;

extern bool mjOpt_Define( char* section, char* key, int type, void* value, 
                char* defaultValue, char* cmdKey, int cmdKeyValue, char* helpString );
extern bool mjOpt_ParseConf( const char* fileName );
extern bool mjOpt_ParseCmd( int argc, char* argv[] );
extern bool mjOpt_SetValue( char* section, char* key, char* value );
extern void mjOpt_HelpString();

#endif

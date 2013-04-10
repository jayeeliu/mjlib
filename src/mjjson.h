#ifndef _MJJSON_H
#define _MJJSON_H

typedef struct mjJSONStr {
    const char* name;
    char*       val;
    int         len;
    int         must;
} mjJSONStr;

typedef struct mjJSONInt {
    const char* name;
    int*        val;
    int         must;
} mjJSONInt;

extern int mjJSON_Parse( const char *input, mjJSONStr strList[], mjJSONInt intList[] );

#endif

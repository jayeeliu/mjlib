#ifndef _VALIDATOR_H
#define _VALIDATOR_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "mjconnb.h"
#include "mjcomm.h"


#define MJLF_MAX_SQL_LENGTH 512
#define MJLF_MAX_TABLE_NAME_LENGTH 32
#define MJLF_MAX_KEY_LENGTH 64
#define MJLF_MAX_VALUE_LENGTH 65535

#define MJLF_VALID_SUCCESS 0
#define MJLF_VALID_FAIL -1

#define ERR_COMMEND_FORMAT_ERROR 50001
#define ERR_TABLE_NAME_LENGTH -50011
#define ERR_TABLE_NAME_ILLEGAL_CHAR -500012
#define ERR_KEY_LENGTH -50021
#define ERR_KEY_ILLEGAL_CHAR -50022
#define ERR_VALUE_READ_FAIL -50031
#define ERR_VALUE_LENGTH -50032

#define ERR_SQL_QUERY_FAILE -51001
#define ERR_SYSTEM_ERROR -52001


extern int is_valid_table_name(mjstr tn);
extern int is_valid_key(mjstr key);
extern int is_valid_value(mjstr value);
extern void mjlf_error(int error_code, char* error_msg);
extern void show_error(int error_code, mjconnb conn);
extern void show_succ(mjconnb conn, char* data);
extern int mjlf_validate(mjstrlist args, unsigned int args_length, mjconnb conn);

#endif

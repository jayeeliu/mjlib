#ifndef _VALIDATOR_H
#define _VALIDATOR_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "mjconnb.h"
#include "mjcomm.h"

#define DPC_MODE_WRITE "w"
#define DPC_MODE_READ "r"

#define MJLF_MAX_SQL_LENGTH 65600
#define MJLF_MAX_TABLE_NAME_LENGTH 32
#define MJLF_MAX_KEY_LENGTH 64
#define MJLF_MAX_VALUE_LENGTH 65535

#define MJLF_VALID_SUCCESS 0
#define MJLF_VALID_FAIL -1

#define ERR_COMMAND_FORMAT_ERROR 50001
#define ERR_COMMAND_NOT_SUPPORT_NOW 50002
#define ERR_SERVER_CONFIG 50031
#define ERR_STORE_CONNECT_FAIL 50032

#define ERR_TABLE_NAME_LENGTH -50011
#define ERR_TABLE_NAME_ILLEGAL_CHAR -500012
#define ERR_KEY_LENGTH -50021
#define ERR_KEY_ILLEGAL_CHAR -50022
#define ERR_VALUE_READ_FAIL -50031
#define ERR_VALUE_LENGTH -50032

#define ERR_SQL_QUERY_FAIL -51001
#define ERR_SYSTEM_ERROR -52001

#define ERR_REDIS_EXEC_FAIL 53001


extern int is_valid_table_name(mjstr tn);
extern int is_valid_key(mjstr key);
extern int is_valid_value(mjstr value);
extern void mjlf_error(int error_code, char* error_msg);
extern void show_error(int error_code, mjconnb conn);
extern void show_succ(mjconnb conn, char* data);
extern int mjlf_validate(mjstrlist args, unsigned int args_length, mjconnb conn);
extern mjstr read_value(mjconnb conn, mjstr len);

#endif

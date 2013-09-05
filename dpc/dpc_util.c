/*
 * params validator
 */

#include "dpc_util.h"

int is_valid_table_name(mjstr tn) {/*{{{*/
  if (tn->length < 1 || tn->length > MJLF_MAX_TABLE_NAME_LENGTH) {
    return ERR_TABLE_NAME_LENGTH;
  }
  int i = 0;
  for (; i<tn->length; i++) {
    // allow lower and upper letter, number and _
    if (!isalnum(tn->data[i]) && tn->data[i] != 95) {
      return ERR_TABLE_NAME_ILLEGAL_CHAR;
    }
  }
  return MJLF_VALID_SUCCESS;
}/*}}}*/

int is_valid_key(mjstr key) {/*{{{*/
  if (key->length < 1 || key->length > MJLF_MAX_KEY_LENGTH) {
    return ERR_KEY_LENGTH;
  }
  return MJLF_VALID_SUCCESS;
}/*}}}*/

int is_valid_value(mjstr value) {/*{{{*/
  if (value->length < 1 || value->length > MJLF_MAX_VALUE_LENGTH) {
    return ERR_VALUE_LENGTH;
  }
  return MJLF_VALID_SUCCESS;
}/*}}}*/

/* error code to error message
 * {{{
 */
void mjlf_error(int error_code, char* error_msg) {
  switch(error_code) {
    case ERR_COMMAND_FORMAT_ERROR:
      sprintf(error_msg, "%d command error. Format: command table [key] [length]\\r\\n[value\\r\\n]\r\n", error_code);
      break;
    case ERR_TABLE_NAME_LENGTH:
      sprintf(error_msg, "%d table name is too short or too long, 1~%d\r\n", error_code, MJLF_MAX_TABLE_NAME_LENGTH);
      break;
    case ERR_TABLE_NAME_ILLEGAL_CHAR:
      sprintf(error_msg, "%d table name contains illegal character(s)\r\n", error_code);
      break;
    case ERR_KEY_LENGTH:
      sprintf(error_msg, "%d the key is too short or too long, 1~%dB\r\n", error_code, MJLF_MAX_KEY_LENGTH);
      break;
    case ERR_VALUE_READ_FAIL:
      sprintf(error_msg, "%d value data read fail\r\n", error_code);
      break;
    case ERR_VALUE_LENGTH:
      sprintf(error_msg, "%d the value is too short or too long, 1~%dB\r\n", error_code, MJLF_MAX_VALUE_LENGTH);
      break;
    case ERR_SQL_QUERY_FAIL:
      sprintf(error_msg, "%d execute fail\r\n", error_code);
      break;
    case ERR_COMMAND_NOT_SUPPORT_NOW:
      sprintf(error_msg, "%d command not support now\r\n", error_code);
      break;
    case ERR_REDIS_EXEC_FAIL:
      sprintf(error_msg, "%d redis fail\r\n", error_code);
      break;
    case ERR_SERVER_CONFIG:
      sprintf(error_msg, "%d server config error\r\n", error_code);
      break;
    case ERR_STORE_CONNECT_FAIL:
      sprintf(error_msg, "%d store service connect fail\r\n", error_code);
      break;
    default:
      sprintf(error_msg, "-50000 unknow error\r\n");
      break;
  }
}/*}}}*/


void show_error(int error_code, mjconnb conn) {/*{{{*/
  char error[256];
  mjlf_error(error_code, error);
  mjconnb_writes(conn, error);
}/*}}}*/

void show_succ(mjconnb conn, char* data) {/*{{{*/
  int len;
  if (data != NULL && (len = strlen(data)) > 0) {
    // 65535 + len("OK ") + 10
    char buf[65538];
    sprintf(buf, "OK %d\r\n%s", len, data);
    mjconnb_writes(conn, buf);
  } else {
    mjconnb_writes(conn, "OK\r\n");
  }
}/*}}}*/

/* validate all request params
 * {{{
 * @param mjstrlist args request params list
 * @param int args_legth current command arguments length, for validating request params
 * @param mjconnb conn   current connection
 */
int mjlf_validate(mjstrlist args, unsigned int args_length, mjconnb conn) {
  int error_code;

  if (args->length != args_length) {
    show_error(ERR_COMMAND_FORMAT_ERROR, conn);
    return MJLF_VALID_FAIL;
  }

  if (args_length >= 2 && (error_code = is_valid_table_name(args->data[1])) != MJLF_VALID_SUCCESS) {
    show_error(error_code, conn);
    return MJLF_VALID_FAIL;
  }

  if (args_length >= 3 && (error_code = is_valid_key(args->data[2])) != MJLF_VALID_SUCCESS) {
    show_error(error_code, conn);
    return MJLF_VALID_FAIL;
  }

#ifdef DEBUGING
  args_length >= 2 && printf("key:%s\r\n", args->data[1]->data);
  args_length >= 3 && printf("value:%s\r\n", args->data[2]->data);
#endif

  if (args_length >= 4) {
    int length = atoi(args->data[3]->data);
    if (length < 1 || length > MJLF_MAX_VALUE_LENGTH) {
      show_error(ERR_VALUE_LENGTH, conn);
      return MJLF_VALID_FAIL;
    }
  }
/* // not execute
  if (args_length >= 5 && (error_code = is_valid_value(args->data[4])) != MJLF_VALID_SUCCESS) {
    show_error(error_code, conn);
    return MJLF_VALID_FAIL;
  }
*/
  return MJLF_VALID_SUCCESS;
}/*}}}*/

/* read put value
 * {{{
 */
mjstr read_value(mjconnb conn, mjstr len) {
  int length = atoi(len->data);
  mjstr content = mjstr_new(1024);
  mjconnb_readbytes(conn, content, length);
  if (length != content->length) {
    mjstr_delete(content);
    show_error(ERR_VALUE_READ_FAIL, conn);
    return NULL;
  }
  return content;
}/*}}}*/



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <mysql/mysql.h>
#include <mysql/errmsg.h>
#include <mysql/mysqld_error.h>
#include "mjlog.h"
#include "mjsql.h"

#define MYSQL_CONN_TIMEOUT      3
#define MYSQL_CHARSET_NAME      "utf8"

struct mjsql_data {
  MYSQL*      msp;
  MYSQL_RES*  result;
  MYSQL_ROW   row;
  int         num_fields;
  int         num_rows;
};
typedef struct mjsql_data* mjsql_data;

/*
===============================================================================
mjsql_conn
    connect to mysql
    return 0 -- fail, 1 -- sucess
===============================================================================
*/
bool mjsql_conn(mjsql handle, int retry) {
  // sanity check
  if (!handle || retry < 0) {
    MJLOG_ERR("handle is null or retry < 0");
    return false;
  }
  // close msp first
  if (handle->data->msp) {
    mysql_close(handle->data->msp);
    handle->data->msp = NULL;
  }
  // init mysql database */
   MYSQL* msp = mysql_init(NULL);
  if (!msp) {
    MJLOG_ERR("mysql init error");
    return false;
  }
  // set connect timeout
  int retval;
  uint32_t timeout = MYSQL_CONN_TIMEOUT;
  retval = mysql_options(msp, MYSQL_OPT_CONNECT_TIMEOUT, 
      (const char*)(&timeout));
  if (retval) {
    MJLOG_ERR("Failed to set option: %s", mysql_error(msp));
    mysql_close(msp);
    return false;
  }
  // set rw timeout
  retval = mysql_options(msp, MYSQL_OPT_READ_TIMEOUT, (const char*)(&timeout));
  if (retval) {
    MJLOG_ERR("Failed to set option: %s", mysql_error(msp));
    mysql_close(msp);
    return false;
  }
  retval = mysql_options(msp, MYSQL_OPT_WRITE_TIMEOUT, (const char*)(&timeout));
  if (retval) {
    MJLOG_ERR("Failed to set option: %s", mysql_error(msp));
    mysql_close(msp);
    return false;
  }
  // set charset to utf-8
  retval = mysql_options(msp, MYSQL_SET_CHARSET_NAME, MYSQL_CHARSET_NAME); 
  if (retval) {
    MJLOG_ERR("Failed to set option: %s", mysql_error(msp));
    mysql_close(msp);
    return false;
  }
  // try to connect db
  while (retry >= 0) {
     if (mysql_real_connect(msp, handle->db_host, handle->db_user, 
          handle->db_pass, handle->db_name, handle->db_port, NULL,
          CLIENT_INTERACTIVE | CLIENT_MULTI_STATEMENTS)) {
      handle->data->msp = msp;
       return true;
    }
    retry--;
    // connect failed 
    MJLOG_ERR("Failed to connect: [%s][%d] %s", handle->db_host, 
        handle->db_port, mysql_error(msp));
  }
  mysql_close(msp);
  return false;
}


/*
===============================================================================
mjsql_query
    mysql query
    return 0 -- success, -1 -- failed
            other -- errnum
===============================================================================
*/
int mjsql_query(mjsql handle, const char* sql_str, int sql_len) {
  /* sanity check */
  if (!handle || !sql_str) {
    MJLOG_ERR("handle or sql_str is null");
    return -1;
  }
  // reconnect database
  if (!handle->data->msp && !mjsql_conn(handle, 0)) {
    MJLOG_ERR("mjsql_query: MySQL can not be reconnected!.");
    return -1;
  }
  // free result
  if (handle->data->result) {
    mysql_free_result(handle->data->result);
    handle->data->result = NULL;
  }
  handle->data->row = NULL;
  handle->data->num_fields = -1;
  handle->data->num_rows = -1;
  // run query
  while (1) {
    int retval = mysql_real_query(handle->data->msp, sql_str, sql_len);
    if (!retval) {
      handle->data->num_rows = mysql_affected_rows(handle->data->msp);
      return 0; // success
    }
    // Here!!! some error happens
    MJLOG_ERR("mysql query error:[%s][%s][%d][%d]:[%s]", sql_str, 
      handle->db_host, handle->db_port, retval, 
      mysql_error(handle->data->msp));
    // get error code
    int err = mysql_errno(handle->data->msp);
    if (err >= ER_ERROR_FIRST && err <= ER_ERROR_LAST) {
      // SQL syntax error
      if (err == ER_SYNTAX_ERROR) MJLOG_ERR("SQL syntax error.");
      return err;
    }
    if (err == CR_SERVER_GONE_ERROR || err == CR_SERVER_LOST) {
      MJLOG_INFO("Try to reconnect to mysql ...");
      // connect database retry 1
      if (!mjsql_conn(handle, 1)) {
        MJLOG_ERR("Fatal error: MySQL can not be reconnected!.");
        break;
      }
      MJLOG_INFO("MySQL reconnected ok!.");
      continue;  
    }
    //  unknow error
    MJLOG_EMERG("Mysql Unknow error. mysql failed.");
    break;
  }
  return -1;
}

/*
===============================================================================
mjsql_store_result
  store result to mjsql
===============================================================================
*/
bool mjsql_store_result(mjsql handle) {
  // sanity check 
  if (!handle || !handle->data->msp) {
    MJLOG_ERR("handle or handle->msp is null");
    return false;
  }
  // get result set
  while (1) {
    handle->data->result = mysql_store_result(handle->data->msp);
    if (handle->data->result) {
      handle->data->num_rows = mysql_affected_rows(handle->data->msp);
      return true;
    }
    // get error number 
    int err = mysql_errno(handle->data->msp);
    if (!err) break;  // no error return 
     // syntax error 
    if (err >= ER_ERROR_FIRST && err <= ER_ERROR_LAST) {
      MJLOG_ERR("mysql_store_result error");
      break;
    }
    // connection error
    if (err == CR_SERVER_GONE_ERROR || err == CR_SERVER_LOST) {
       MJLOG_INFO("Try to reconnect to mysql ...");
      if (!mjsql_conn(handle, 1)) {
         MJLOG_ERR("Fatal error: MySQL can not be reconnected!.");
         break;
      }
      MJLOG_INFO("MySQL reconnected ok!.");
      continue;
     } 
    // other fatal error 
     MJLOG_EMERG("Mysql Unknow error. mysql failed.");
    break;
  }
   return false;
}

bool mjsql_next_row(mjsql handle) {
  // sanity check
  if (!handle) {
    MJLOG_ERR("mjsql handle is null");
    return false;
  }
  if (!handle->data->result && !mjsql_store_result(handle)) {
    MJLOG_ERR("mjsql_store_result error");
    return false;
  }
  handle->data->num_fields = mysql_num_fields(handle->data->result);
  handle->data->row = mysql_fetch_row(handle->data->result);
  if (!handle->data->row) return false;
  return true;
}

char* mjsql_fetch_row_field(mjsql handle, unsigned int field_num) {
  if (!handle) {
    MJLOG_ERR("mjsql handle is null");
    return NULL;
  }
  if (field_num >= handle->data->num_fields) {
    MJLOG_ERR("field_num outof index");
    return NULL;
  }
  if (!handle->data->row) {
    MJLOG_ERR("row is null");
    return NULL;
  }
  return handle->data->row[field_num];
}

int mjsql_get_fields_num(mjsql handle) {
  if (!handle) {
    MJLOG_ERR("mjsql handle is null");
    return -1;
  }
  return handle->data->num_fields;
}

int mjsql_get_rows_num(mjsql handle) {
  if (!handle) {
    MJLOG_ERR("mjsql handle is null");
    return -1;
  }
  return handle->data->num_rows;
}

/*
===============================================================================
mjsql_real_escape_string
  mjsql escape string
===============================================================================
*/
long long mjsql_real_escape_string(mjsql handle, char *to, const char *from, 
        unsigned long to_len, unsigned long from_len) {
  // sanity check
  if (!handle) {
    MJLOG_ERR("handle is null");
    return -1;
   }
  if (!handle->data->msp && !mjsql_conn(handle, 0)) {
     MJLOG_ERR("MySQL can not be reconnected!.");
    return -1;
  }
  // alloc size
  size_t const new_len = from_len * 2 + 1;
  if (to_len < new_len) {
    char *new_alloc = calloc(1, new_len);
    if (new_alloc == NULL) {
      MJLOG_ERR("data alloc error");
       return -1;
    }
    mysql_real_escape_string(handle->data->msp, new_alloc, from, from_len);
    size_t cp_len = strlen(new_alloc);
    strncpy(to, new_alloc, to_len - 1);
    to[to_len -1] = '\0';
    free(new_alloc);
    return cp_len;
   } 
  mysql_real_escape_string(handle->data->msp, to, from, from_len);
  return 0;
}

/*
===============================================================================
mjsql_new
  create new mysql handler
===============================================================================
*/
mjsql mjsql_new(const char* db_host, const char* db_user, const char* db_pass, 
    const char* db_name, unsigned int db_port) {
  // sanity check
  if (!db_host || !db_user || !db_pass || !db_name) {
    MJLOG_ERR("sanity check error");
    return NULL;
  }
  // alloc mjsql struct
  mjsql handle = (mjsql) calloc(1, sizeof(struct mjsql));
  if (!handle) {
    MJLOG_ERR("mjsql alloc error");
    return NULL;
   }
  // not connect
  handle->data = (mjsql_data) calloc(1, sizeof(struct mjsql_data));
  if (!handle->data) {
    MJLOG_ERR("mjsql_data alloc error");
    free(handle);
    return NULL;
  }
  handle->data->msp         = NULL;    
  handle->data->result      = NULL;
  handle->data->row         = NULL;
  handle->data->num_fields  = -1;
  // store parameter
  strncpy(handle->db_host, db_host, MAX_NAME_LEN);
  strncpy(handle->db_user, db_user, MAX_NAME_LEN);
  strncpy(handle->db_pass, db_pass, MAX_NAME_LEN);
  strncpy(handle->db_name, db_name, MAX_NAME_LEN);
  handle->db_port = db_port;
  // try to connect
  if (!mjsql_conn(handle, 0)) MJLOG_ERR("mjsql connect error");
  // return handle
  return handle;
}

/*
===============================================================================
mjsql_delete
  delete mjsql handler
===============================================================================
*/
bool mjsql_delete(mjsql handle) {
  if (!handle) {
    MJLOG_ERR("handle is null");
    return false;
   }
  if (handle->data->result) mysql_free_result(handle->data->result); 
  if (handle->data->msp) mysql_close(handle->data->msp);
  free(handle->data);
  free(handle);
  return true;
}

#ifndef _MYSQL_CONN_H
#define _MYSQL_CONN_H

#include <mysql/mysql.h>

#define MYSQL_CONN_NAME_LEN		128

struct mjsql {
    MYSQL*          msp;                                /* handle to mysql */
    char            db_host[MYSQL_CONN_NAME_LEN + 1];   /* host name */
    char            db_user[MYSQL_CONN_NAME_LEN + 1];   /* user name */
    char            db_pass[MYSQL_CONN_NAME_LEN + 1];   /* password */
    char            db_name[MYSQL_CONN_NAME_LEN + 1];   /* db name */
    unsigned int    db_port;                            /* port */
};
typedef struct mjsql* mjsql;

extern int          mjsql_conn( mjsql handle, int retry );
extern int          mjsql_query( mjsql handle, const char* sql_str, 
                        int sql_len);
extern long long    mjsql_real_escape_string(mjsql handle, char* to, 
                        const char* from, unsigned long to_len, 
                        unsigned long from_len );
extern MYSQL_RES*   mjsql_store_result( mjsql handle );
extern MYSQL_ROW    mjsql_fetch_row( MYSQL_RES* result, unsigned int* num_fields );
extern void         mjsql_free_result( MYSQL_RES* result );
extern int          mjsql_options( mjsql handle, int option, const void* arg );
extern int          mjsql_set_server_option( mjsql handle, 
                                    enum enum_mysql_set_option option);

extern mjsql        mjsql_new( const char* db_host, const char* db_user, 
                        const char* db_pass, const char* db_name, 
                        unsigned int db_port );  
extern int          mjsql_delete( mjsql handle );

#endif

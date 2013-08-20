#ifndef _MJSQL_H
#define _MJSQL_H

#include <stdbool.h>

#define MAX_NAME_LEN		128

struct mjsql_data;

struct mjsql {
		struct mjsql_data*	data;
    char            		db_host[MAX_NAME_LEN + 1]; 	// host name
    char            		db_user[MAX_NAME_LEN + 1]; 	// user name 
    char            		db_pass[MAX_NAME_LEN + 1]; 	// password 
    char            		db_name[MAX_NAME_LEN + 1]; 	// db name 
    unsigned int    		db_port;                  	// port 
};
typedef struct mjsql* mjsql;

extern bool      	mjsql_conn(mjsql handle, int retry);
extern int       	mjsql_query(mjsql handle, const char* sql_str, int sql_len);
extern long long 	mjsql_real_escape_string(mjsql handle, char* to, 
                  		const char* from, unsigned long to_len, unsigned long from_len);

extern bool				mjsql_next_row(mjsql handle);
extern char*			mjsql_fetch_row_field(mjsql handle, unsigned int field_num);

extern mjsql     	mjsql_new(const char* db_host, const char* db_user, 
                  		const char* db_pass, const char* db_name, unsigned int db_port);  
extern bool      	mjsql_delete(mjsql handle);

#endif

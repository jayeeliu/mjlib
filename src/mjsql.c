#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <mysql/errmsg.h>
#include <mysql/mysqld_error.h>
#include "mjlog.h"
#include "mjsql.h"

#define MYSQL_CONN_TIMEOUT      3
#define MYSQL_CHARSET_NAME      "utf8"

/*
==========================================
mjsql_conn
    connect to mysql
    return 0 -- fail, 1 -- sucess
==========================================
*/
int mjsql_conn( mjsql handle, int retry )
{
    /* sanity check */
    if ( !handle || retry < 0 ) {
        MJLOG_ERR( "handle is null or retry < 0" );
        return 0;
    }

    // close msp first
    if ( handle->msp ) {
        mysql_close( handle->msp );
        handle->msp = NULL;
    }
    
    /* init mysql database */
    MYSQL* msp  = mysql_init( NULL );
    if ( !msp ) {
        MJLOG_ERR( "mysql init error" );
        return 0;
    }

    /* set connect timeout */
    int retval;
    uint32_t timeout = MYSQL_CONN_TIMEOUT;
    
    retval = mysql_options( msp, MYSQL_OPT_CONNECT_TIMEOUT, 
                    ( const char* )( &timeout ) );
    if ( retval ) {
        MJLOG_ERR( "Failed to set option: %s", mysql_error( msp ) );
        mysql_close( msp );
        return 0;
    }

    /* set rw timeout */
    retval = mysql_options( msp, MYSQL_OPT_READ_TIMEOUT, 
                    ( const char* )( &timeout ) );
    if ( retval ) {
        MJLOG_ERR( "Failed to set option: %s", mysql_error( msp ) );
        mysql_close( msp );
        return 0;
    }

    retval = mysql_options( msp, MYSQL_OPT_WRITE_TIMEOUT, 
                    ( const char* )( &timeout ) );
    if ( retval ) {
        MJLOG_ERR( "Failed to set option: %s", mysql_error( msp ) );
        mysql_close( msp );
        return 0;
    }
    
    /* set charset to utf-8 */
    retval = mysql_options( msp, MYSQL_SET_CHARSET_NAME, MYSQL_CHARSET_NAME ); 
    if ( retval ) {
        MJLOG_ERR( "Failed to set option: %s", mysql_error( msp ) );
        mysql_close( msp );
        return 0;
    }

    while ( retry >= 0 ) {
        /* try to connect db */
        if ( mysql_real_connect( msp, handle->db_host, handle->db_user,
                handle->db_pass, handle->db_name, handle->db_port, NULL,
                CLIENT_INTERACTIVE | CLIENT_MULTI_STATEMENTS ) ) {
            handle->msp = msp;
            return 1;
        }

        retry--;
        /* connect failed */
        MJLOG_ERR( "Failed to connect: [%s][%d] %s", handle->db_host, 
            handle->db_port, mysql_error( msp ) );
    }
  
    mysql_close( msp );
    return 0;
}


/*
==========================================
mjsql_query
    mysql query
    return 0 -- success, -1 -- failed
            other -- errnum
==========================================
*/
int mjsql_query( mjsql handle, const char* sql_str, int sql_len )
{
    /* sanity check */
    if ( !handle || !sql_str ) {
        MJLOG_ERR( "handle or sql_str is null" );
        return -1;
    }

    /* reconnect database */
    if ( !handle->msp && !mjsql_conn( handle, 0 ) ) {
        MJLOG_ERR( "mjsql_query: MySQL can not be reconnected!." );
        return -1;
    }

    while ( 1 ) {
        /* run query */
        int retval = mysql_real_query( handle->msp, sql_str, sql_len );
        if ( !retval ) return 0;     /* success */

        /* Here!!! some error happens */
        MJLOG_ERR( "mysql query error:[%s][%s][%d][%d]:[%s]", sql_str, 
                handle->db_host, handle->db_port, retval, 
                mysql_error( handle->msp ) );

        /* get error code */
        int err = mysql_errno( handle->msp );
        if ( err >= ER_ERROR_FIRST && err <= ER_ERROR_LAST ) {
            /* SQL syntax error*/
            if ( err == ER_SYNTAX_ERROR ) MJLOG_ERR( "SQL syntax error." );
            return err;
        }
        if ( err == CR_SERVER_GONE_ERROR || err == CR_SERVER_LOST ) {
            MJLOG_INFO( "Try to reconnect to mysql ..." );
            /* connect database */
            if ( !mjsql_conn( handle, 1 ) ) {
                MJLOG_ERR( "Fatal error: MySQL can not be reconnected!." );
                break;
            }
            MJLOG_INFO( "MySQL reconnected ok!." );
            continue;
        }

        /* unknow error */
        MJLOG_EMERG( "Mysql Unknow error. mysql failed." );
        break;
    }
    return -1;
}


MYSQL_RES* mjsql_store_result( mjsql handle )
{
    /* sanity check */
    if ( !handle || !handle->msp ) {
        MJLOG_ERR("handle or handle->msp is null");
        return NULL;
    }

    while ( 1 ) {
        /* get result set */
        MYSQL_RES* result = mysql_store_result( handle->msp );
        if ( result ) return result;
        
        /* get error number */
        int err = mysql_errno( handle->msp );
        if ( !err ) break;  /* no error return */
        
        /* syntax error */
        if ( err >= ER_ERROR_FIRST && err <= ER_ERROR_LAST ) {
            MJLOG_ERR( "mysql_store_result error" );
            break;
        }
        
        /* connection error */
        if ( err == CR_SERVER_GONE_ERROR || err == CR_SERVER_LOST ) {
            MJLOG_INFO( "Try to reconnect to mysql ..." );
            if ( !mjsql_conn( handle, 1 ) ) {
                MJLOG_ERR( "Fatal error: MySQL can not be reconnected!." );
                break;
            }
            MJLOG_INFO( "MySQL reconnected ok!." );
            continue;
        } 
            
        /* other fatal error */
        MJLOG_EMERG( "Mysql Unknow error. mysql failed." );
        break;
    }

    return NULL;
}

MYSQL_ROW mjsql_fetch_row( MYSQL_RES* result, unsigned int* num_fields )
{
    if ( !result ) return NULL;

    *num_fields = mysql_num_fields( result );
    return mysql_fetch_row( result );
}

void mjsql_free_result( MYSQL_RES* result )
{
    if ( !result ) return;
    mysql_free_result( result );
}

/* set mysql options */
int mjsql_options( mjsql handle, int option, const void* arg )
{
    /* sanity check */
    if ( !handle || !arg ) {
        MJLOG_ERR( "handle or arg is null" );
        return -1;
    }
    /* connect to db */
    if ( !handle->msp && !mjsql_conn( handle, 0 ) ) {
        MJLOG_ERR( "mysql_options: MySQL can not be reconnected!." );
        return -1;
    }
    /* set mysql options */
    mysql_options( handle->msp, option, arg );
    return 0;
}


/* set mysql server options */
int mjsql_set_server_option( mjsql handle, enum enum_mysql_set_option option )
{
    if ( !handle ) {
        MJLOG_ERR( "handle is null" );
        return -1;
    }
    if ( !handle->msp && !mjsql_conn( handle, 0 ) ) {
        MJLOG_ERR( "mysql_server_option: MySQL can not be reconnected!." );
        return -1;
    }
    mysql_set_server_option( handle->msp, option );
    return 0;
}

long long mjsql_real_escape_string(mjsql handle, char *to, const char *from, 
        unsigned long to_len, unsigned long from_len)
{
    if (!handle) {
        MJLOG_ERR("handle is null");
        return -1;
    }

    if (!handle->msp && !mjsql_conn(handle, 0)) {
        MJLOG_ERR("__mysql_options: MySQL can not be reconnected!.");
        return -1;
    }

    size_t const new_len = from_len * 2 + 1;
    if (to_len < new_len) {
        char *new_alloc = calloc(1, new_len);
        if(new_alloc == NULL) {
            MJLOG_ERR("data alloc error");
            return -1;
        }

        mysql_real_escape_string(handle->msp, new_alloc, from, from_len);
        size_t cp_len = strlen(new_alloc);
        strncpy(to, new_alloc, to_len - 1);
        to[to_len -1] = '\0';
        free(new_alloc);
        return cp_len;
    } 
   
    mysql_real_escape_string(handle->msp, to, from, from_len);
    return 0;
}

mjsql mjsql_new( const char* db_host, const char* db_user, 
        const char* db_pass, const char* db_name, unsigned int db_port )
{
    /* sanity check */
    if ( !db_host || !db_user || !db_pass || !db_name ) {
        MJLOG_ERR( "sanity check error" );
        return NULL;
    }

    mjsql handle = ( mjsql ) calloc( 1, sizeof( struct mjsql ) );
    if ( !handle ) {
        MJLOG_ERR( "mjsql alloc error" );
        return NULL;
    }

    handle->msp = NULL;    
    /* store parameter */
    strncpy( handle->db_host, db_host, MYSQL_CONN_NAME_LEN );
    strncpy( handle->db_user, db_user, MYSQL_CONN_NAME_LEN );
    strncpy( handle->db_pass, db_pass, MYSQL_CONN_NAME_LEN );
    strncpy( handle->db_name, db_name, MYSQL_CONN_NAME_LEN );
    handle->db_port = db_port;

    /* connect the database */
    if ( !mjsql_conn( handle, 0 ) ) {
        MJLOG_ERR( "mjsql connect error" );
    }

    return handle;
}

int mjsql_delete( mjsql handle ) 
{
	if ( !handle ) {
        MJLOG_ERR( "handle is null" );
        return -1;
    }
    if ( handle->msp ) {	
        mysql_close( handle->msp );
    }
    free( handle );
	return 0;
}

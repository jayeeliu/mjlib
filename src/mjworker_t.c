#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include "mjsock.h"
#include "unistd.h"
#include "mjworker.h"
#include "mjlog.h"
#include "mjconnb.h"
#include "mjsql.h"

void myproc( void* arg )
{
    mjConnB conn = ( mjConnB )arg;
    mjstr data = mjstr_new();

    mjConnB_ReadUntil( conn, "\r\n\r\n", data );

    //connect to sql server
   // mjsql handle = mjsql_new( "127.0.0.1", "root", "", "test", 3306 );
   // char* str = "select * from tt";
   // mjsql_query( handle, str, strlen( str ) );
  //  MYSQL_RES* result = mjsql_store_result( handle );
   // mjsql_free_result( result );
   // mjsql_delete( handle );

   // mjstr_copys( data, "OK\n" ); 
    mjConnB_WriteS( conn, "TCP FORK SERVER READY!" );

    mjstr_delete(data);
}

int main()
{
    int sfd = mjsock_tcpserver( 7879 );
    if ( sfd < 0 ) {
        MJLOG_ERR( "create socket error" );
        return 1;
    }
    
    WorkerRun( 2, 2, sfd, myproc ); 
    return 0;
}

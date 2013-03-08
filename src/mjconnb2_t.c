#include <stdio.h>

#include "mjstr.h"
#include "mjconnb.h"

int main()
{
    mjConnB conn = mjConnB_NewClient( "mail.sina.com.cn", 80, 3000);
    if ( !conn ) {
        printf( "conn error" );
        return -1;
    }
    mjstr data = mjstr_new();
    mjstr_copys( data, "GET / HTTP/1.1\r\n\r\n" );
    mjConnB_Write( conn, data );
    mjConnB_Read( conn, data );
    
    printf( "%s", data->str );

    mjstr_delete( data );
    mjConnB_Delete( conn );

    return 0;
}

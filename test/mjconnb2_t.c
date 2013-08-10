#include <stdio.h>

#include "mjstr.h"
#include "mjconnb.h"

int main()
{
    mjconnb conn = mjconnb_connect( "mail.sina.com.cn", 80, 3000);
    if ( !conn ) {
        printf( "conn error" );
        return -1;
    }
    mjStr data = mjStr_New();
    mjStr_CopyS( data, "GET / HTTP/1.1\r\n\r\n" );
    mjconnb_write( conn, data );
    mjconnb_read( conn, data );
    
    printf( "%s", data->data );

    mjStr_Delete( data );
    mjconnb_delete( conn );

    return 0;
}

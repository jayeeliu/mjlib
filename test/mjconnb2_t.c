#include <stdio.h>

#include "mjstr.h"
#include "mjconnb.h"

int main()
{
    mjconnb conn = mjconnb_Connect( "mail.sina.com.cn", 80, 3000);
    if ( !conn ) {
        printf( "conn error" );
        return -1;
    }
    mjStr data = mjStr_New();
    mjStr_CopyS( data, "GET / HTTP/1.1\r\n\r\n" );
    mjconnb_Write( conn, data );
    mjconnb_Read( conn, data );
    
    printf( "%s", data->data );

    mjStr_Delete( data );
    mjconnb_Delete( conn );

    return 0;
}

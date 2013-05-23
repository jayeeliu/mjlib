#include <stdio.h>
#include "mjio.h"
#include "mjlog.h"

int main()
{
    mjIO io = mjIO_New( "test.ini" );
    if ( !io ) {
        MJLOG_ERR( "mjIO_New error" );
        return 1;
    }

    mjStr data = mjStr_New();
    mjIO_Read( io, data, 10 );
    printf( "%s\n", data->data );
    mjIO_Read( io, data, 10 );
    printf( "%s\n", data->data );
    mjIO_Read( io, data, 10 );
    printf( "%s\n", data->data );
    mjStr_Delete( data );
    return 0;
}

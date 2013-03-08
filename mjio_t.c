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

    mjstr data = mjstr_new();
    mjIO_ReadLine( io, data );
    printf( "%s\n", data->str );
    mjIO_ReadLine( io, data );
    printf( "%s\n", data->str );
    mjIO_ReadLine( io, data );
    printf( "%s\n", data->str );
    return 0;
}

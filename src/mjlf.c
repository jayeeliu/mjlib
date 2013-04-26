#include <stdlib.h>
#include "mjlog.h"
#include "mjlf.h"

mjLF mjLF_New( mjProc Routine, int threadNum )
{
    mjLF server = ( mjLF ) calloc( 1, sizeof( struct mjLF ) );
    if ( !server ) {
        MJLOG_ERR( "server create errror" );
        return NULL;
    }

    return server;
}

#include "mjproto_txt.h"
#include "mjlog.h"

bool mjTxt_RunCmd( PROTO_TXT_ROUTINE routineList[], mjConnB conn )
{
    mjStr data = mjStr_New();
    if ( !data ) {
        MJLOG_ERR( "data create error" );
        return false;  
    }
    mjConnB_ReadUntil( conn, "\r\n", data );

    mjStrList strList = mjStrList_New();
    if ( !strList ) {
        MJLOG_ERR( "mjstrlist create error" );
        return false;
    }

    mjStr_Split( data, " ", strList );
    if ( strList->length < 3 ) {
        mjConnB_WriteS( conn, "+command error\r\n" );
        return false;
    }

    mjStr tag = mjStrList_Get( strList, 0 );
    mjStr cmd = mjStrList_Get( strList, 1 );

    mjConnB_WriteS( conn, "+tag: " );
    mjConnB_Write( conn, tag );
    mjConnB_WriteS( conn, " cmd: " );
    mjConnB_Write( conn, cmd );
    mjConnB_WriteS( conn, "\r\n" );

    mjStrList_Delete( strList );
    mjStr_Delete( data );
    return true;
}

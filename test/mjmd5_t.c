#include <stdio.h>
#include "mjmd5.h"

int main() {
    unsigned char digest[16];
    mjMD5( "", digest );
    for( int i = 0; i < 16; i++ ) {
        printf( "%X ", digest[i] );
    }
    printf( "\n" );
    return 0;
}

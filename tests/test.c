#include <stdio.h>
#include <stdlib.h>

#include "../tomlib.h"

int main( int argc, char* argv[], char** envp )
{
    char* file = NULL;
    if( argc > 1 )
        file = argv[1];
    toml_key_t* toml = toml_load( file );
    if( toml==NULL )
        return 1;
    toml_json_dump( toml );
    toml_free( toml );
    return 0;
}

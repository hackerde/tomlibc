#include <stdio.h>
#include <stdlib.h>

#include "../src/toml.h"

int main( int argc, char* argv[], char** envp )
{
    char* file = NULL;
    if( argc > 1 )
        file = argv[1];
    key_t* toml = toml_load( file );
    if( toml==NULL )
        return 1;
    json_dump( toml );
    delete_key( toml );
    return 0;
}

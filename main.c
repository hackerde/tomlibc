#include <stdio.h>
#include <stdlib.h>

#include "tomlib.h"

int main( int argc, char* argv[], char** envp )
{
    /* Load TOML data from a file */
    const char* filename = "sample.toml";
    toml_key_t* toml = toml_load( filename );
    if( !toml )
    {
        printf( "Could not load toml file: %s\n", filename );
    }
    toml_key_t* k;
    k = toml_get_key( toml_get_key( toml_get_key( toml, "data" ), "constants" ), "max" );
    if( !k )
    {
        printf( "Key data.constants.max does not exist!\n" );
        exit(1);
    }
    if( !( k->value->type==FLOAT ) )
    {
        printf( "Key data.constants.max is not a float!\n" );
        exit(1);
    }
    double* max = k->value->data;
    if( 100000000 < *max )
    {
        k = toml_get_key( toml_get_key( toml, "data" ), "d3" );
        if( !k )
        {
            printf( "Key data.d3 does not exist!\n" );
            exit(1);
        }
        if( !( k->value->type==ARRAY ) )
        {
            printf( "Key data.d3 is not an array!\n" );
            exit(1);
        }
        toml_value_t* v = k->value->arr[1];
        if( v && v->type==INLINETABLE )
        {
            k = toml_get_key( ( toml_key_t* )v->data, "email" );
            if( k && k->value->type==STRING )
            {
                printf( "Email: %s\n", ( char* )k->value->data );
            }
        }
    }
    /* Free up the memory after doing stuff */
    toml_free( toml );
    return 0;
}

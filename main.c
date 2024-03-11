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
        exit( 1 );
    }
    double* max = toml_get_float(
                  toml_get_key(
                  toml_get_key(
                  toml_get_key( toml, "data" ),
                                      "constants" ),
                                      "max" ) );
    if( !max )
    {
        printf( "data.constants.max is not a float\n" );
        exit( 1 );
    }
    if( 100000000 < *max )
    {
        toml_value_t* v = toml_get_array(
                          toml_get_key(
                          toml_get_key( toml, "data" ),
                                              "d3" ) );
        if( v && v->len>1 )
        {
            if( v->arr[ 1 ]->type==TOML_INLINETABLE &&
                v->arr[ 1 ]->data )
            {
                char* email = toml_get_string(
                              toml_get_key( v->arr[ 1 ]->data, "email" ) );
                if( email ) printf( "Email: %s\n", email );
            }
        }
    }
    /* Free up the memory after doing stuff */
    toml_free( toml );
    return 0;
}

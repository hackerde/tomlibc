#include "key.h"
#include "value.h"

#include <string.h>
#include <stdlib.h>
#include <time.h>

toml_value_t* new_string( const char* s )
{
    toml_value_t* v = calloc( 1, sizeof( toml_value_t ) );
    v->type = STRING;
    v->data = calloc( 1, strlen( s )+1 );
    memcpy( v->data, s, strlen( s ) );
    return v;
}

toml_value_t* new_number(
    double*         d,
    toml_value_type_t    type,
    size_t          precision,
    bool            scientific
)
{
    toml_value_t* v = calloc( 1, sizeof( toml_value_t ) );
    v->type = type;
    v->data = calloc( 1, sizeof( double ) );
    memcpy( v->data, d, sizeof( double ) );
    v->scientific = scientific;
    v->precision = precision;
    return v;
}

toml_value_t* new_datetime(
    struct tm*      dt,
    toml_value_type_t    type,
    char*           format
)
{
    toml_value_t* v = calloc( 1, sizeof( toml_value_t ) );
    v->type = type;
    v->data = calloc( 1, sizeof( struct tm ) );
    v->format = calloc( 1, strlen( format ) );
    memcpy( v->format, format, strlen( format ) );
    memcpy( v->data, dt, sizeof( struct tm ) );
    return v;
}

toml_value_t* new_array()
{
    toml_value_t* v = calloc( 1, sizeof( toml_value_t ) );
    v->type = ARRAY;
    v->arr = calloc( 1, sizeof( toml_value_t* )*MAX_ARRAY_LENGTH );
    return v;
}

toml_value_t* new_inline_table( toml_key_t* k )
{
    toml_value_t* v = calloc( 1, sizeof( toml_value_t ) );
    v->type = INLINETABLE;
    toml_key_t* h = new_key( KEY );
    for( toml_key_t** iter=k->subkeys; iter<k->last; iter++ )
        add_subkey( h, *iter );
    v->data = h;
    return v;
}

void delete_value( toml_value_t* v )
{
    if( !v ) return;
    if( v->arr )
    {
        for( toml_value_t** iter=v->arr; *iter!=NULL; iter++ )
            delete_value( *iter );
        free( v->arr );
    }
    if( v->data )
        free( v->data );
    free( v );
}

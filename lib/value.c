#include "key.h"
#include "value.h"

#include <string.h>
#include <stdlib.h>
#include <time.h>

value_t* new_string( const char* s )
{
    value_t* v = calloc( 1, sizeof( value_t ) );
    v->type = STRING;
    v->data = calloc( 1, strlen( s ) );
    memcpy( v->data, s, strlen( s ) );
    return v;
}

value_t* new_number(
    double*         d,
    value_type_t    type,
    size_t          precision,
    bool            scientific
)
{
    value_t* v = calloc( 1, sizeof( value_t ) );
    v->type = type;
    v->data = calloc( 1, sizeof( double ) );
    memcpy( v->data, d, sizeof( double ) );
    v->scientific = scientific;
    v->precision = precision;
    return v;
}

value_t* new_datetime(
    struct tm*      dt,
    value_type_t    type,
    char*           format
)
{
    value_t* v = calloc( 1, sizeof( value_t ) );
    v->type = type;
    v->data = calloc( 1, sizeof( struct tm ) );
    v->format = format;
    memcpy( v->data, dt, sizeof( struct tm ) );
    return v;
}

value_t* new_array()
{
    value_t* v = calloc( 1, sizeof( value_t ) );
    v->type = ARRAY;
    v->arr = calloc( 1, sizeof( value_t* )*MAX_ARRAY_LENGTH );
    return v;
}

value_t* new_inline_table( key_t* k )
{
    value_t* v = calloc( 1, sizeof( value_t ) );
    v->type = INLINETABLE;
    key_t* h = new_key( KEY );
    for( key_t** iter=k->subkeys; iter<k->last; iter++ )
        add_subkey( h, *iter );
    v->data = h;
    return v;
}

void delete_value( value_t* v )
{
    if( !v ) return;
    if( v->arr )
    {
        for( value_t** iter=v->arr; *iter!=NULL; iter++ )
            delete_value( *iter );
        free( v->arr );
    }
    if( v->data )
        free( v->data );
    free( v );
}

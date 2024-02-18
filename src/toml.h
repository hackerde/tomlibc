#ifndef __SRC_TOML_H__
#define __SRC_TOML_H__

#include "../lib/key.h"
#include "../lib/value.h"
#include "../lib/utils.h"
#include "keys.h"
#include "values.h"

#include <string.h>
#include <math.h>

#define ESCAPE( C )         \
    if( C=='\b' )           \
    {                       \
        printf( "\\b" );    \
        continue;           \
    }                       \
    if( C=='\n' )           \
    {                       \
        printf( "\\n" );    \
        continue;           \
    }                       \
    if( C=='\r' )           \
    {                       \
        printf( "\\r" );    \
        continue;           \
    }                       \
    if( C=='\t' )           \
    {                       \
        printf( "\\t" );    \
        continue;           \
    }                       \
    if( C=='\f' )           \
    {                       \
        printf( "\\f" );    \
        continue;           \
    }                       \
    if( C=='\\' )           \
    {                       \
        printf( "\\\\" );   \
        continue;           \
    }                       \
    if( C=='\"' )           \
    {                       \
        printf( "\\\"" );   \
        continue;           \
    }

key_t* toml_load( const char* file )
{
    key_t* root = new_key( TABLE );
    memcpy( root->id, "root", 5 );

    FILE* stream;
    if( file )
    {
        stream = fopen( file, "r" );
        FAIL_RETURN( stream, "Could not open file: %s\n", file )
    }
    else
    {
        stream = stdin;
        file = "stdin";
    }
    tokenizer_t* tok = new_tokenizer( stream );
    next_token( tok );

    key_t* key = root;
    while( has_token( tok )!=0 )
    {
        key = parse_keyval( tok, key, root );
        FAIL_RETURN( key, "Encountered an error while parsing %s\n"
                          "At line %d column %d\n",
                          file, tok->line, tok->col
        )
    }

    return root;
}

void dump_key   ( key_t* k );
void dump_value ( value_t* v );
void dump_toml  ( key_t* root );

void dump_key( key_t* k )
{
    if( k->type==KEYLEAF && k->value!=NULL && k->value->type!=INLINETABLE )
    {
        printf( "\"" );
        for( char* c=k->id; *c!= '\0'; c++ )
        {
            ESCAPE( *c )
            printf( "%c", *c );
        }
        printf( "\": " );
        dump_value( k->value );
    }
    else if( k->type==ARRAYTABLE )
    {
        printf( "\"" );
        for( char* c=k->id; *c!= '\0'; c++ )
        {
            ESCAPE( *c )
            printf( "%c", *c );
        }
        printf( "\": [\n" );
        for( size_t i=0; i<=k->idx; i++ )
        {
            dump_value( k->value->arr[ i ] );
            if( i!=k->idx )
                printf( ",\n" );
        }
        printf("\n]");
    }
    else
    {
        printf( "\"" );
        for( char* c=k->id; *c!= '\0'; c++ )
        {
            ESCAPE( *c )
            printf( "%c", *c );
        }
        printf( "\": {\n" );
        for( key_t** iter=k->subkeys; iter<k->last; iter++ )
        {
            dump_key( *iter );
            if( ( iter+1 )!=k->last )
                printf( ",\n" );
        }
        printf( "\n}" );
    }
}

void dump_value( value_t* v )
{
    switch ( v->type )
    {
        case STRING:
        {
            printf( "{\"type\": \"string\", \"value\": \"" );
            for( char* c=( char* )( v->data ); *c!= '\0'; c++ )
            {
                ESCAPE( *c )
                printf( "%c", *c );
            }
            printf( "\"}" );
            break;
        }
        case FLOAT:
        {
            printf( "{\"type\": \"float\", \"value\": " );
            double f = *( double* )( v->data );
            if( f==( double ) INFINITY )
                printf( "\"inf\"}" );
            else if( f==( double ) -INFINITY )
                printf( "\"-inf\"}" );
            else if( isnan(f) )
                printf( "\"nan\"}" );
            else if( v->scientific )
                printf( "\"%g\"}", f );
            else if( f==0.0 )
                printf( "\"0.0\"}" );
            else
                printf( "\"%.*lf\"}", ( int )v->precision, f );
            break;
        }
        case INT:
        {
            printf( "{\"type\": \"integer\", \"value\": " );
            printf( "\"%.0lf\"}", *( double* )( v->data ) );
            break;
        }
        case BOOL:
        {
            printf( "{\"type\": \"bool\", \"value\": " );
            if( *( double* )( v->data ) )
                printf( "\"true\"}" );
            else
                printf( "\"false\"}" );
            break;
        }
        case DATETIME:
        {
            printf( "{\"type\": \"datetime\", \"value\": " );
            char buf[ 255 ] = { 0 };
            strftime( buf, sizeof( buf ), v->format, ( struct tm* )v->data );
            printf( "\"%s\"}", buf);
            break;
        }
        case DATETIMELOCAL:
        {
            printf( "{\"type\": \"datetime-local\", \"value\": " );
            char buf[ 255 ] = { 0 };
            strftime( buf, sizeof( buf ), v->format, ( struct tm* )v->data );
            printf( "\"%s\"}", buf );
            break;
        }
        case DATELOCAL:
        {
            printf( "{\"type\": \"date-local\", \"value\": " );
            char buf[ 255 ] = { 0 };
            strftime( buf, sizeof( buf ), v->format, ( struct tm* )v->data );
            printf( "\"%s\"}", buf );
            break;
        }
        case TIMELOCAL:
        {
            printf( "{\"type\": \"time-local\", \"value\": " );
            char buf[ 255 ] = { 0 };
            strftime( buf, sizeof( buf ), v->format, ( struct tm* )v->data );
            printf( "\"%s\"}", buf );
            break;
        }
        case ARRAY:
        {
            printf( "[\n" );
            for( value_t** iter=v->arr; *iter!=NULL; iter++ )
            {
                dump_value( *iter );
                if( *( iter+1 )!=NULL )
                    printf( ",\n" );
            }
            printf("\n]");
            break;
        }
        case INLINETABLE:
        {
            printf( "{\n" );
            key_t* k = ( key_t* )( v->data );
            for( key_t** iter=k->subkeys; iter<k->last; iter++ )
            {
                dump_key( ( *iter ) );
                if( ( iter+1 )!=k->last )
                    printf( ",\n" );
            }
            printf( "\n}" );
            break;
        }
        default:
            printf( "\nprint_value: unknown value type\n" );
            break;
    }
}

void json_dump( key_t* root )
{
    printf( "{\n" );
    for( key_t** iter=root->subkeys; iter<root->last; iter++ )
    {
        dump_key( *iter );
        if( (iter+1)!=root->last )
            printf( ",\n" );
    }
    printf( "\n}\n" );
}

#endif

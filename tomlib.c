#include "tomlib.h"

#include "parser/lib/tokenizer.h"
#include "parser/lib/utils.h"
#include "parser/lib/key.h"

#include "parser/parse_keys.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

toml_key_t* toml_load( const char* file )
{
    toml_key_t* root = new_key( TOML_TABLE );
    memcpy( root->id, "root", strlen( "root" ) );

    FILE* stream;
    if( file )
    {
        stream = fopen( file, "r" );
        RETURN_IF_FAILED( stream, "Could not open file: %s\n", file );
    }
    else
    {
        stream = stdin;
        file = "stdin";
    }
    // Max file size - 1GB
    #define MAX_FILE_SIZE 1073741824
    tokenizer_t* tok = new_tokenizer( stream );
    RETURN_IF_FAILED( get_input_size( tok )<MAX_FILE_SIZE,
                      "Input from %s is too big\n", file );
    next_token( tok );

    toml_key_t* key = root;
    while( has_token( tok )!=0 )
    {
        key = parse_keyval( tok, key, root );
        RETURN_IF_FAILED( key, "Encountered an error while parsing %s\n"
                          "At line %d column %d\n",
                          file, tok->line+1, tok->col );
    }

    delete_tokenizer( tok );

    return root;
}

toml_key_t* toml_get_key(
    toml_key_t* key,
    const char* id
)
{
    if( key==NULL )
    {
        return NULL;
    }
    if( strcmp( key->id, id )==0 )
    {
        return key;
    }
    for( toml_key_t** iter=key->subkeys; iter<key->last; iter++ )
    {
        if( strcmp( ( *iter )->id, id )==0 )
        {
            return *iter;
        }
    }
    LOG_ERR( "node %s does not exist in subkeys of node %s", id, key->id );
    return NULL;
}

char* toml_get_string( toml_key_t* key )
{
    if( !key )                               return NULL;
    if( !( key->value ) )                    return NULL;
    if( !( key->value->type==TOML_STRING ) ) return NULL;
    if( !( key->value->data ) )              return NULL;
    return ( char* )( key->value->data );
}

int* toml_get_int( toml_key_t* key )
{
    if( !key )                               return NULL;
    if( !( key->value ) )                    return NULL;
    if( !( key->value->type==TOML_INT ) )    return NULL;
    if( !( key->value->data ) )              return NULL;
    return ( int* )( key->value->data );
}

double* toml_get_float( toml_key_t* key )
{
    if( !key )                               return NULL;
    if( !( key->value ) )                    return NULL;
    if( !( key->value->type==TOML_FLOAT ) )  return NULL;
    if( !( key->value->data ) )              return NULL;
    return ( double* )( key->value->data );
}

bool* toml_get_bool( toml_key_t* key )
{
    if( !key )                               return NULL;
    if( !( key->value ) )                    return NULL;
    if( !( key->value->type==TOML_BOOL ) )   return NULL;
    if( !( key->value->data ) )              return NULL;
    return ( bool* )( key->value->data );
}

struct tm* toml_get_datetime( toml_key_t* key )
{
    if( !key )                               return NULL;
    if( !( key->value ) )                    return NULL;
    if( !( key->value->type==TOML_DATETIME      ||
           key->value->type==TOML_DATETIMELOCAL ||
           key->value->type==TOML_DATELOCAL     ||
           key->value->type==TOML_TIMELOCAL
         )
    ) return NULL;
    if( !( key->value->data ) )              return NULL;
    return ( struct tm* )( key->value->data );
}

toml_value_t* toml_get_array( toml_key_t* key )
{
    if( !key )                               return NULL;
    if( !( key->value ) )                    return NULL;
    if( !( key->value->type==TOML_ARRAY ) )  return NULL;
    return key->value;
}

static inline void string_dump( const char* s )
{
    for( const char* c=s; *c!='\0'; c++ )
    {
        switch ( *c )
        {
            case '\b':
                printf( "\\b" );
                continue;
            case '\n':
                printf( "\\n" );
                continue;
            case '\r':
                printf( "\\r" );
                continue;
            case '\t':
                printf( "\\t" );
                continue;
            case '\f':
                printf( "\\f" );
                continue;
            case '\\':
                printf( "\\\\" );
                continue;
            case '"':
                printf( "\\\"" );
                continue;
            default:
                break;
        }
        printf( "%c", *c );
    }
}

void toml_key_dump( toml_key_t* k )
{
    if( k->type==TOML_KEYLEAF && k->value!=NULL && k->value->type!=TOML_INLINETABLE )
    {
        printf( "\"" );
        string_dump( k->id );
        printf( "\": " );
        toml_value_dump( k->value );
    }
    else if( k->type==TOML_ARRAYTABLE )
    {
        printf( "\"" );
        string_dump( k->id );
        printf( "\": [\n" );
        for( size_t i=0; i<=k->idx; i++ )
        {
            toml_value_dump( k->value->arr[ i ] );
            if( i!=k->idx )
            {
                printf( ",\n" );
            }
        }
        printf("\n]");
    }
    else
    {
        printf( "\"" );
        string_dump( k->id );
        printf( "\": {\n" );
        for( toml_key_t** iter=k->subkeys; iter<k->last; iter++ )
        {
            toml_key_dump( *iter );
            if( ( iter+1 )!=k->last )
            {
                printf( ",\n" );
            }
        }
        printf( "\n}" );
    }
}

void toml_value_dump( toml_value_t* v )
{
    switch ( v->type )
    {
        case TOML_STRING:
        {
            printf( "{\"type\": \"string\", \"value\": \"" );
            string_dump( ( char* )v->data );
            printf( "\"}" );
            break;
        }
        case TOML_FLOAT:
        {
            printf( "{\"type\": \"float\", \"value\": " );
            double f = *( double* )( v->data );
            if( f==( double ) INFINITY )
            {
                printf( "\"inf\"}" );
            }
            else if( f==( double ) -INFINITY )
            {
                printf( "\"-inf\"}" );
            }
            else if( isnan(f) )
            {
                printf( "\"nan\"}" );
            }
            else if( v->scientific )
            {
                printf( "\"%g\"}", f );
            }
            else if( f==0.0 )
            {
                printf( "\"0.0\"}" );
            }
            else
            {
                printf( "\"%.*lf\"}", ( int )v->precision, f );
            }
            break;
        }
        case TOML_INT:
        {
            printf( "{\"type\": \"integer\", \"value\": " );
            printf( "\"%.0lf\"}", *( double* )( v->data ) );
            break;
        }
        case TOML_BOOL:
        {
            printf( "{\"type\": \"bool\", \"value\": " );
            if( *( double* )( v->data ) )
            {
                printf( "\"true\"}" );
            }
            else
            {
                printf( "\"false\"}" );
            }
            break;
        }
        case TOML_DATETIME:
        {
            printf( "{\"type\": \"datetime\", \"value\": " );
            char buf[ 255 ] = { 0 };
            strftime( buf, sizeof( buf ), v->format, ( struct tm* )v->data );
            printf( "\"%s\"}", buf);
            break;
        }
        case TOML_DATETIMELOCAL:
        {
            printf( "{\"type\": \"datetime-local\", \"value\": " );
            char buf[ 255 ] = { 0 };
            strftime( buf, sizeof( buf ), v->format, ( struct tm* )v->data );
            printf( "\"%s\"}", buf );
            break;
        }
        case TOML_DATELOCAL:
        {
            printf( "{\"type\": \"date-local\", \"value\": " );
            char buf[ 255 ] = { 0 };
            strftime( buf, sizeof( buf ), v->format, ( struct tm* )v->data );
            printf( "\"%s\"}", buf );
            break;
        }
        case TOML_TIMELOCAL:
        {
            printf( "{\"type\": \"time-local\", \"value\": " );
            char buf[ 255 ] = { 0 };
            strftime( buf, sizeof( buf ), v->format, ( struct tm* )v->data );
            printf( "\"%s\"}", buf );
            break;
        }
        case TOML_ARRAY:
        {
            printf( "[\n" );
            for( toml_value_t** iter=v->arr; *iter!=NULL; iter++ )
            {
                toml_value_dump( *iter );
                if( *( iter+1 )!=NULL )
                {
                    printf( ",\n" );
                }
            }
            printf("\n]");
            break;
        }
        case TOML_INLINETABLE:
        {
            printf( "{\n" );
            toml_key_t* k = ( toml_key_t* )( v->data );
            for( toml_key_t** iter=k->subkeys; iter<k->last; iter++ )
            {
                toml_key_dump( ( *iter ) );
                if( ( iter+1 )!=k->last )
                {
                    printf( ",\n" );
                }
            }
            printf( "\n}" );
            break;
        }
        default:
            printf( "\nprint_value: unknown value type\n" );
            break;
    }
}

void toml_json_dump( toml_key_t* root )
{
    printf( "{\n" );
    for( toml_key_t** iter=root->subkeys; iter<root->last; iter++ )
    {
        toml_key_dump( *iter );
        if( (iter+1)!=root->last )
        {
            printf( ",\n" );
        }
    }
    printf( "\n}\n" );
}

void toml_free( toml_key_t* toml )
{
    delete_key( toml );
}

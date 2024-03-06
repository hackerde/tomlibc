#include "tomlib.h"

#include "parser/lib/tokenizer.h"
#include "parser/lib/utils.h"
#include "parser/lib/key.h"

#include "parser/parse_keys.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

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

toml_key_t* toml_load( const char* file )
{
    toml_key_t* root = new_key( TABLE );
    memcpy( root->id, "root", strlen( "root" ) );

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

    toml_key_t* key = root;
    while( has_token( tok )!=0 )
    {
        key = parse_keyval( tok, key, root );
        FAIL_RETURN( key, "Encountered an error while parsing %s\n"
                          "At line %d column %d\n",
                          file, tok->line+1, tok->col
        )
    }

    delete_tokenizer( tok );

    return root;
}

void toml_key_dump( toml_key_t* k )
{
    if( k->type==KEYLEAF && k->value!=NULL && k->value->type!=INLINETABLE )
    {
        printf( "\"" );
        string_dump( k->id );
        printf( "\": " );
        toml_value_dump( k->value );
    }
    else if( k->type==ARRAYTABLE )
    {
        printf( "\"" );
        string_dump( k->id );
        printf( "\": [\n" );
        for( size_t i=0; i<=k->idx; i++ )
        {
            toml_value_dump( k->value->arr[ i ] );
            if( i!=k->idx )
                printf( ",\n" );
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
                printf( ",\n" );
        }
        printf( "\n}" );
    }
}

void toml_value_dump( toml_value_t* v )
{
    switch ( v->type )
    {
        case STRING:
        {
            printf( "{\"type\": \"string\", \"value\": \"" );
            string_dump( ( char* )v->data );
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
            for( toml_value_t** iter=v->arr; *iter!=NULL; iter++ )
            {
                toml_value_dump( *iter );
                if( *( iter+1 )!=NULL )
                    printf( ",\n" );
            }
            printf("\n]");
            break;
        }
        case INLINETABLE:
        {
            printf( "{\n" );
            toml_key_t* k = ( toml_key_t* )( v->data );
            for( toml_key_t** iter=k->subkeys; iter<k->last; iter++ )
            {
                toml_key_dump( ( *iter ) );
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

void toml_json_dump( toml_key_t* root )
{
    printf( "{\n" );
    for( toml_key_t** iter=root->subkeys; iter<root->last; iter++ )
    {
        toml_key_dump( *iter );
        if( (iter+1)!=root->last )
            printf( ",\n" );
    }
    printf( "\n}\n" );
}

toml_key_t* toml_get_key(
    toml_key_t* key,
    const char* id
)
{
    if( key==NULL )
        return NULL;
    if( strcmp( key->id, id )==0 )
        return key;
    for( toml_key_t** iter=key->subkeys; iter<key->last; iter++ )
    {
        if( strcmp( ( *iter )->id, id )==0 )
            return *iter;
    }
    LOG_ERR_RETURN( "node %s does not exist in subkeys of node %s", id, key->id );
}

void toml_free( toml_key_t* root )
{
    delete_key( root );
}
#ifndef __SRC_TOML_H__
#define __SRC_TOML_H__

// NOTE (AJB): Header hygiene is super important in C. Headers are textual inclusions, so any file that does '#include "toml.h"' will
// get everything in this file as if they wrote it themselves. There's no public/private. There's no namespacing of any kind. If you
// want to understand how wild this ends up being.. write a simple "Hello, World" program and run `gcc -E`. That will expand all includes and
// all defines. You can witness the horror.
//
// A such, the established practice is to minimize what ends up in the header and "hide" things in the source files whenever possible.
// Things like the ESCAPE macro is especially dangerous. Very common name, likely to collide with something a user might write.
// Taking firedancer as an example (I haven't read much of it but Kevin's C style is permenantly etched into my brain). This is why, you
// see name prefixing of EVERYTHING: fd_blah_t, FD_MACRO_THING, etc, etc. Anything exposed in this file (incl. recursively), I'd prefix with something like "toml_"

// NOTE (AJB): really not sure the distinction btwn lib and src.. especially if you're including lib/ headers anyways
#include "../lib/key.h"
#include "../lib/value.h"
#include "../lib/utils.h"
#include "keys.h"
#include "values.h"

#include <string.h>
#include <math.h>

// NOTE (AJB) As above, you really don't want a macro like this in a public exported library header. A common practice is to have "internal" or "private"
// headers and public headers. That way you can define common macros and utility functions without poluting the public header namespace. See fd_whatever_private.h for an
// example of how KB does stuff like that. His approach is fairly non-standard but I'm also biased to it.

// NOTE(AJB) You may also want to learn about hygienic macros. Macors in C are utter dogshit. But it's what we have, so there are techniques people use to make them less
// dangerous. E.g. "do { STUFF_HERE } while(0)". But, in this particular case, I'd suggest just using a "static inline" function. Generally should prefer "static inline"
// functions over macros unless you have a really good reason not to: e.g. compiler won't generate the right machine code otherwise, or code has to run in the same function
// context. For example: "#define LIKELY(cond) __builtin_expect(!!(cond), 0)"
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

// NOTE (AJB): Never define bare functions in headers. Either (1) declare prototypes only, or (2) only define "static inline" functions. Doing this will lead to multiple-definition link-time errors. Like this:
//        app.c
//        /   \
//     foo.c  bar.c
//       |     |
//     toml.h  toml.h
//
// Then both foo.c and bar.c have defined key_t* toml_load( const char* file ) and the linker doesn't know what to do
// In some languages (couch C++) .. this can lead to some complicated structure that "successfully links" and crashes
// in awful and cryptic ways later (I've seen it in code of people you know...). Ideally, put this in a C file, and only
// have prototyes in this header. Hide any definitions/declarations you don't need for the header.
key_t* toml_load( const char* file )
{
    key_t* root = new_key( TABLE );
    memcpy( root->id, "root", 5 ); // NOTE(AJB): Not a bug.. but just some "fear of god".. anytime you do this or see this, you should think.. "could this possibly buffer-overflow?" This attitude may save you. Again this is not a bug.. just teaching you proper C paranoia.

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
        // NOTE(AJB) Seems like you just want a basic helper function to print the id
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

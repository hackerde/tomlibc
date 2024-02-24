#include "keys.h"
#include "values.h"
#include "utils.h"

#include "../lib/key.h"
#include "../lib/value.h"
#include "../lib/utils.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>

#define VALUE_INIT( SIZE )      \
    char value[ SIZE ];         \
    memset( value, 0, SIZE );

#define PARSE_DT_FORMAT( FORMATS, TYPE )                                    \
    for( size_t i=0; i<sizeof( FORMATS )/sizeof( char* ); i++ )             \
    {                                                                       \
        char* e = strptime( value, FORMATS[ i ], time );                    \
        if( e!=NULL )                                                       \
        {                                                                   \
            dt = calloc( 1, sizeof( datetime_t ) );                         \
            dt->format = calloc( 1, strlen( FORMATS[ i ] ) );               \
            memcpy( dt->format, FORMATS[ i ], strlen( FORMATS[ i ] ) );     \
            dt->type = TYPE;                                                \
            dt->dt = time;                                                  \
            break;                                                          \
        }                                                                   \
        memset( time, 0, sizeof( struct tm ) );                             \
    }                                                                       \
    if( dt ) return dt;

char* parse_basicstring( tokenizer_t* tok, char* value )
{
    size_t idx = 0;
    bool multi = false;
    while( has_token( tok ) )
    {
        FAIL_BREAK( idx<MAX_STRING_LENGTH, "buffer overflow\n" );
        if( is_basicstringstart( get_token( tok ) ) )
        {
            if( idx > 0 && !multi )
            {
                // some chars parsed, not multiline, seen "
                next_token( tok );
                return value;
            }
            else if( idx==0 && !multi )
            {
                // no chars parsed, seen ""
                next_token( tok );
                if( is_whitespace( get_token( tok ) ) || is_newline( get_token( tok ) ) )
                    return value;
                FAIL_BREAK( is_basicstringstart( get_token( tok ) ), "cannot start string with 2 double-quotes\n" )
                multi = true;
            }
            else if( multi )
            {
                // seen """
                size_t a = next_token( tok );
                size_t b = next_token( tok );
                if( is_basicstringstart( get_token( tok ) ) && is_basicstringstart( get_prev( tok ) ) )
                {
                    next_token( tok );
                    return value;
                }
                else
                {
                    value[ idx++ ] = '"';
                    backtrack( tok, a+b-1 );
                    continue;
                }
            }
        }
        else if( is_newline( get_token( tok ) ) && !multi )
            LOG_ERR_BREAK( "newline before end of string\n" )
        else if( is_newline( get_token( tok ) ) && multi && idx==0 )
            ;
        else if( is_escape( get_token( tok ) ) )
        {
            next_token( tok );
            char c = escape( get_token( tok ) );
            if( multi && c==0 )
            {
                bool hit = false;
                while( is_whitespace( get_token( tok ) ) || is_newline( get_token( tok ) ) )
                {
                    if( is_whitespace( get_token( tok ) ) )
                        parse_whitespace( tok );
                    if( is_newline( get_token( tok ) ) )
                    {
                        hit = true;
                        next_token( tok );
                    }
                }
                FAIL_BREAK( hit, "cannot have characters on same line after \\\n" )
                continue;
            }
            else
            {
                FAIL_BREAK( c!=0, "unknown escape sequence \\%c\n", get_token( tok ) )
                value[ idx++ ] = c;
            }
        }
        else
            value[ idx++ ] = get_token( tok );
        next_token( tok );
    }
    return NULL;
}

char* parse_literalstring( tokenizer_t* tok, char* value )
{
    size_t idx = 0;
    bool multi = false;
    while( has_token( tok ) )
    {
        FAIL_BREAK( idx<MAX_STRING_LENGTH, "buffer overflow\n" );
        if( is_literalstringstart( get_token( tok ) ) )
        {
            if( idx > 0 && !multi )
            {
                // some chars parsed, not multiline, seen '
                next_token( tok );
                return value;
            }
            else if( idx==0 && !multi )
            {
                // no chars parsed, seen ''
                next_token( tok );
                if( is_whitespace( get_token( tok ) ) || is_newline( get_token( tok ) ) )
                    return value;
                FAIL_BREAK( is_literalstringstart( get_token( tok ) ), "cannot start string with 2 single-quotes\n" )
                multi = true;
            }
            else if( multi )
            {
                // seen '''
                next_token( tok );
                bool next = next_token( tok );
                if( is_literalstringstart( get_token( tok ) ) && is_literalstringstart( get_prev( tok ) ) )
                {
                    next_token( tok );
                    return value;
                }
                else
                {
                    value[ idx++ ] = '\'';
                    if( next )
                        backtrack( tok, 1 );
                    else
                        LOG_ERR_BREAK( "reached end of file before end of string\n" )
                    continue;
                }
            }
        }
        else if( is_newline( get_token( tok ) ) && !multi )
            LOG_ERR_BREAK( "newline before end of string\n" )
        else if( is_newline( get_token( tok ) ) && multi && idx==0 )
            ;
        else
            value[ idx++ ] = get_token( tok );
        next_token( tok );
    }
    return NULL;
}

datetime_t* parse_datetime(
    tokenizer_t*    tok,
    char*           value,
    const char*     num_end
)
{
    datetime_t* dt = NULL;
    size_t idx = 0;
    // check to allow only 1 whitespace character
    int spaces = 0;
    char* datetimes[] = {
        "%Y-%m-%dT%H:%M:%SZ",
        "%Y-%m-%dT%H:%M:%S%Z",
        "%Y-%m-%dT%H:%M:%S%z",
        "%Y-%m-%d %H:%M:%SZ",
        "%Y-%m-%d %H:%M:%S%Z",
        "%Y-%m-%d %H:%M:%S%z"
    };
    char* datetimelocals[] = {
        "%Y-%m-%dT%H:%M:%S",
        "%Y-%m-%d %H:%M:%S"
    };
    char* datelocals[] = { "%Y-%m-%d" };
    char* timelocals[] = { "%H:%M:%S" };
    while( has_token( tok ) )
    {
        FAIL_BREAK( idx<MAX_STRING_LENGTH, "buffer overflow\n" );
        if( ( is_whitespace( get_token( tok ) ) && spaces ) ||
            ( !is_whitespace( get_token( tok ) ) && is_numberend( get_token( tok ), num_end ) ) )
        {
            struct tm* time = calloc( 1, sizeof( struct tm ) );
            PARSE_DT_FORMAT( datetimes, DATETIME )
            PARSE_DT_FORMAT( datetimelocals, DATETIMELOCAL )
            PARSE_DT_FORMAT( datelocals, DATELOCAL )
            PARSE_DT_FORMAT( timelocals, TIMELOCAL )
            LOG_ERR_BREAK( "could not parse %s as datetime\n", value )
        }
        else
        {
            if( is_whitespace( get_token( tok ) ) )
                spaces++;
            value[ idx++ ] = get_token( tok );
        }
        next_token( tok );
    }
    return NULL;
}

double parse_inf_nan( tokenizer_t* tok, bool negative )
{
    double ret = 0.0;
    if( get_token( tok )=='i' )
    {
        next_token( tok );
        next_token( tok );
        if( get_prev( tok )=='n' && get_token( tok )=='f' )
        {
            if( negative )
                ret = ( double ) -INFINITY;
            else
                ret = ( double ) INFINITY;
        }
    }
    if( get_token( tok )=='n' )
    {
        next_token( tok );
        next_token( tok );
        if( get_prev( tok )=='a' && get_token( tok )=='n' )
        {
            if( negative )
                ret = ( double ) -NAN;
            else
                ret = ( double ) NAN;
        }
    }
    next_token( tok );
    return ret;
}

value_t* parse_array( tokenizer_t* tok, value_t* arr )
{
    size_t idx = 0;
    bool sep = true;
    while( has_token( tok ) )
    {
        FAIL_BREAK( idx<MAX_ARRAY_LENGTH, "buffer overflow\n" );
        if( is_arrayend( get_token( tok ) ) )
        {
            next_token( tok );
            return arr;
        }
        else if( is_arraysep( get_token( tok ) ) )
        {
            FAIL_BREAK( !sep, "expected value but got , instead\n" )
            sep = true;
            next_token( tok );
        }
        else if( is_newline( get_token( tok ) ) )
            next_token( tok );
        else if( is_whitespace( get_token( tok ) ) )
            parse_whitespace( tok );
        else if( is_commentstart( get_token( tok ) ) )
            parse_comment( tok );
        else
        {
            FAIL_BREAK( sep, "expected , between elements\n" )
            value_t* v = parse_value( tok, "#,] \n" );
            FAIL_BREAK( v, "could not parse value\n" )
            arr->arr[ idx++ ] = v;
            sep = false;
        }
    }
    return NULL;
}

double parse_boolean( tokenizer_t* tok )
{
    double ret = 2.0;
    if( get_token( tok )=='t' )
    {
        next_token( tok );
        next_token( tok );
        next_token( tok );
        if( get_prev_prev( tok )!='r' || get_prev( tok )!='u' || get_token( tok )!='e' )
            ret = 2.0;
        else
            ret = 1.0;
    }
    else if( get_token( tok )=='f' )
    {
        next_token( tok );
        next_token( tok );
        next_token( tok );
        if( get_prev_prev( tok )!='a' || get_prev( tok )!='l' || get_token( tok )!='s' )
            ret = 2.0;
        else {
            next_token( tok );
            if( get_token( tok )=='e' )
                ret = 0.0;
        }
    }
    next_token( tok );
    return ret;
}

key_t* parse_inlinetable( tokenizer_t* tok )
{
    key_t* keys = new_key( TABLE );
    bool sep = true;
    while( has_token( tok ) )
    {
        if( is_inlinetableend( get_token( tok ) ) )
        {
            FAIL_BREAK( !sep, "cannot have trailing comma in inline table\n" )
            next_token( tok );
            return keys;
        }
        else if( is_inlinetablesep( get_token( tok ) ) )
        {
            FAIL_BREAK( !sep, "expected key-value but got , instead" )
            sep = true;
            next_token( tok );
        }
        else if( is_newline( get_token( tok ) ) )
            LOG_ERR_BREAK( "found newline in inline table\n" )
        else if( is_whitespace( get_token( tok ) ) )
            parse_whitespace( tok );
        else
        {
            FAIL_BREAK( sep, "expected , between elements\n" )
            key_t* k = parse_key( tok, keys, true );
            FAIL_BREAK( k, "failed to parse key\n" )
            value_t* v = parse_value( tok, ", }" );
            FAIL_BREAK( v, "failed to parse value\n" )
            k->value = v;
            parse_whitespace( tok );
            sep = false;
        }
    }
    return NULL;
}

void parse_comment( tokenizer_t* tok )
{
    while( has_token( tok ) )
    {
        next_token( tok );
        // TODO: can be optimized by doing a getline
        if( is_newline( get_token( tok ) ) )
        {
            next_token( tok );
            break;
        }
    }
}

void parse_whitespace( tokenizer_t* tok )
{
    while( has_token( tok ) )
    {
        if( !is_whitespace( get_token( tok ) ) )
            break;
        next_token( tok );
    }
}

// TODO: unused
char* parse_escape( tokenizer_t* tok )
{
    char* escaped = calloc( 1, 4*sizeof( char ) );
    switch ( get_token( tok ) )
    {
        case 'b':
        {
            escaped[0] = '\b';
            return escaped;
        }
        case 't':
        {
            escaped[0] = '\t';
            return escaped;
        }
        case 'n':
        {
            escaped[0] = '\n';
            return escaped;
        }
        case 'f':
        {
            escaped[0] = '\f';
            return escaped;
        }
        case 'r':
        {
            escaped[0] = '\r';
            return escaped;
        }
        case '"':
        {
            escaped[0] = '\"';
            return escaped;
        }
        case '\\':
        {
            escaped[0] = '\\';
            return escaped;
        }
        case 'u':
        {
            size_t digits = 0;
            char code[ 9 ] = { 0 };
            next_token( tok );
            while( has_token( tok ) )
            {
                if( digits>8 )
                    break;
                if( is_hexdigit( get_token( tok ) ) || is_digit( get_token( tok ) ) )
                    code[ digits++ ] = get_token( tok );
                else
                {
                    if( digits!=4 || digits!=8 )
                        break;
                    char* end;
                    unsigned long num = strtoul( code, &end, 16 );
                    if( code==end )
                        break;
                    // %x80-D7FF / %xE000-10FFFF
                    // return actual escape
                    if( ( num>=0x80 && num<=0xD7FF ) || ( num>=0xE000 && num<=0x10FFFF ) )
                    {
                        escaped[0] = (num >> 24) & 0xFF;
                        escaped[1] = (num >> 16) & 0xFF;
                        escaped[2] = (num >> 8) & 0xFF;
                        escaped[3] = num & 0xFF;
                        return escaped;
                    }
                    else
                        break;
                }
            }
            LOG_ERR_RETURN( "Invalid unicode escape code\n" )
        }
        default:
            LOG_ERR_RETURN( "Unknown escape sequence: \\%c\n", get_token( tok) )
    }
}

double parse_base_uint(
    tokenizer_t*    tok,
    int             base,
    char*           value,
    const char*     num_end
)
{
    size_t idx = 0;
    double d = -1;
    while( has_token( tok ) )
    {
        FAIL_BREAK( idx<MAX_STRING_LENGTH, "buffer overflow\n" );
        if( is_numberend( get_token( tok ), num_end ) )
        {
            FAIL_BREAK( idx!=0, "incomplete non-decimal number" )
            char* end;
            unsigned long num = strtoul( value, &end, base );
            if( end==value+idx )
                d = ( double ) num;
            else
                LOG_ERR( "could not convert %s to base %d\n", value, base );
            break;
        }
        else if( is_underscore( get_token( tok ) ) )
        {
            next_token( tok );
            if( ( is_digit( get_token( tok ) ) || ( base==16 && is_hexdigit( get_token( tok ) ) ) ) &&
                ( is_digit( get_prev_prev( tok ) ) || ( base==16 && is_hexdigit( get_prev_prev( tok ) ) ) ) )
                value[ idx++ ] = get_token( tok );
            else
                LOG_ERR_BREAK( "stray %c character\n", get_prev( tok ) );
        }
        else
            value[ idx++ ] = get_token( tok );
        next_token( tok );
    }
    return d;
}

number_t* parse_number(
    tokenizer_t*    tok,
    char*           value,
    double*         d,
    const char*     num_end
)
{
    size_t idx = 0;
    number_t* n = calloc( 1, sizeof( number_t ) );
    n->type = INT;
    n->scientific = false;
    n->precision = 0;
    while( has_token( tok ) )
    {
        FAIL_BREAK( idx<MAX_STRING_LENGTH, "buffer overflow\n" )
        if( is_numberend( get_token( tok ), num_end ) )
        {
            char* end;
            double num = strtod( value, &end );
            FAIL_RETURN( end==value+idx, "could not convert %s to double\n", value )
            *d = num;
            if( n->precision>0 ) n->precision--;
            if( n->type==INT && num!=0 )
            {
                FAIL_RETURN( value[0]!='0', "cannot have leading zero for integers" )
                if( value[0]=='+' || value[0]=='-' )
                    FAIL_RETURN( value[1]!='0', "cannot have leading zero for signed integers" )
            }
            return n;
        }
        else if( idx==0 && get_token( tok )=='0' )
        {
            double b;
            next_token( tok );
            if( get_token( tok )=='x' )
            {
                // hexadecimal
                next_token( tok );
                b = parse_base_uint( tok, 16, value, num_end );
            }
            else if( get_token( tok )=='o' )
            {
                // octal
                next_token( tok );
                b = parse_base_uint( tok, 8, value, num_end );
            }
            else if( get_token( tok )=='b' )
            {
                // binary
                next_token( tok );
                b = parse_base_uint( tok, 2, value, num_end );
            }
            else
            {
                value[ idx++ ] = '0';
                continue;
            }
            FAIL_RETURN( b!=-1, "invalid non-decimal number\n" )
            *d = b;
            return n;
        }
        else if( is_decimalpoint( get_token( tok ) ) || is_underscore( get_token( tok ) ) )
        {
            if( is_decimalpoint( get_token( tok ) ) )
            {
                value[ idx++ ] = get_token( tok );
                n->type = FLOAT;
                n->precision = 1;
            }
            next_token( tok );
            if( is_digit( get_token( tok ) ) && is_digit( get_prev_prev( tok ) ) )
            {
                if( n->precision>0 ) n->precision++;
                value[ idx++ ] = get_token( tok );
            }
            else
                LOG_ERR_RETURN( "stray %c character\n", get_prev( tok ) )
        }
        else if( get_token( tok )=='i' || get_token( tok )=='n' )
        {
            if( idx==1 && ( get_prev( tok )=='+' || get_prev( tok )=='-') )
            {
                double f = parse_inf_nan( tok, ( get_prev( tok )=='-' ) );
                if( f==0 )
                    break;
                *d = f;
                n->type = FLOAT;
                n->precision = 0;
                return n;
            }
            else
                LOG_ERR_RETURN( "unknown or invalid number\n" )
        }
        else if( get_token( tok )=='x' || get_token( tok )=='X' ||
                 get_token( tok )=='b' || get_token( tok )=='B' ||
                 get_token( tok )=='o' || get_token( tok )=='O' )
            LOG_ERR_BREAK( "invalid decimal number, found stray character %c\n", get_token( tok ) )
        else
        {
            value[ idx++ ] = get_token( tok );
            if( n->precision>0 ) n->precision++;
            if( get_token( tok )=='e' || get_token( tok )=='E' )
            {
                n->type = FLOAT;
                n->scientific = true;
            }
        }
        next_token( tok );
    }
    char* end;
    double num = strtod( value, &end );
    FAIL_RETURN( end==value+idx, "could not convert %s to double\n", value )
    *d = num;
    if( n->precision>0 ) n->precision--;
    if( n->type==INT && num!=0 )
    {
        FAIL_RETURN( value[0]!='0', "cannot have leading zero for integers" )
        if( value[0]=='+' || value[0]=='-' )
            FAIL_RETURN( value[1]!='0', "cannot have leading zero for signed integers" )
    }
    return n;
}

value_t* parse_value(
    tokenizer_t*    tok,
    const char*     num_end
)
{
    while( has_token( tok ) )
    {
        FAIL_BREAK( !is_newline( get_token( tok ) ), "got a newline before any value\n" )
        if( is_whitespace( get_token( tok ) ) )
        {
            parse_whitespace( tok );
            continue;
        }
        else if( is_basicstringstart( get_token( tok ) ) )
        {
            VALUE_INIT( MAX_STRING_LENGTH )
            next_token( tok );
            char* s = parse_basicstring( tok, value );
            FAIL_BREAK( s, "could not parse basic string\n" )
            value_t* v = new_string( value );
            return v;
        }
        else if( is_literalstringstart( get_token( tok ) ) )
        {
            VALUE_INIT( MAX_STRING_LENGTH )
            next_token( tok );
            char* s = parse_literalstring( tok, value );
            FAIL_BREAK( s, "could not parse literal string\n" )
            value_t* v = new_string( value );
            return v;
        }
        else if( is_numberstart( get_token( tok ) ) )
        {
            VALUE_INIT( MAX_STRING_LENGTH )
            // try parsing date time
            bool a = next_token( tok );
            bool b = next_token( tok );
            if( has_token( tok ) && get_token( tok )==':' )
            {
                backtrack( tok, a+b );
                datetime_t* dt = parse_datetime( tok, value, num_end );
                FAIL_BREAK( dt, "could not parse time\n" )
                value_t* v = new_datetime( dt->dt, dt->type, dt->format );
                return v;
            }
            else
            {
                bool c = next_token( tok );
                bool d = next_token( tok );
                if( has_token( tok ) && get_token( tok )=='-' )
                {
                    backtrack( tok, a+b+c+d );
                    datetime_t* dt = parse_datetime( tok, value, num_end );
                    FAIL_BREAK( dt, "could not parse datetime\n" )
                    value_t* v = new_datetime( dt->dt, dt->type, dt->format );
                    return v;
                }
                else
                    backtrack( tok, a+b+c+d );
            }
            double* d = calloc( 1, sizeof( double ) );
            number_t* n = parse_number( tok, value, d, num_end );
            FAIL_BREAK( n, "could not parse number\n" )
            value_t* v = new_number( d, n->type, n->precision, n->scientific );
            return v;
        }
        else if( is_arraystart( get_token( tok ) ) )
        {
            value_t* v = new_array();
            next_token( tok );
            v = parse_array( tok, v );
            FAIL_BREAK( v, "could not parse array\n" )
            return v;
        }
        else if( is_inlinetablestart( get_token( tok ) ) )
        {
            next_token( tok );
            key_t* keys = parse_inlinetable( tok );
            FAIL_BREAK( keys, "could not parse inline table\n" )
            value_t* v = new_inline_table( keys );
            return v;
        }
        else if( get_token( tok )=='t' || get_token( tok )=='f' )
        {
            double b = parse_boolean( tok );
            FAIL_BREAK( ( b==1 || b==0 ), "expecting true or false but could not parse\n" )
            value_t* v = new_number( &b, BOOL, 0, false );
            return v;
        }
        else if( get_token( tok )=='i' || get_token( tok )=='n' )
        {
            double f = parse_inf_nan( tok, false );
            FAIL_BREAK( f, "expecting inf or nan but could not parse\n" )
            value_t* v = new_number( &f, FLOAT, 0, false );
            return v;
        }
        else
            LOG_ERR_BREAK( "unknown value type\n" )
    }
    return NULL;
}

#include "keys.h"
#include "values.h"
#include "utils.h"

#include "../lib/key.h"
#include "../lib/value.h"
#include "../lib/utils.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>

char* parse_basicstring( tokenizer_t* tok, char* value, bool multi )
{
    size_t idx = 0;
    while( has_token( tok ) )
    {
        FAIL_BREAK( idx<MAX_STRING_LENGTH, "buffer overflow\n" );
        if( is_basicstringstart( get_token( tok ) ) )
        {
            if( !multi )
            {
                next_token( tok );
                return value;
            }
            else
            {
                int a = next_token( tok );
                int b = next_token( tok );
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
        else if( parse_newline( tok ) && !multi )
            LOG_ERR_BREAK( "newline before end of string\n" )
        else if( parse_newline( tok ) && multi && idx==0 )
            ;
        else if( is_escape( get_token( tok ) ) )
        {
            next_token( tok );
            char escaped[5] = { 0 };
            int c = parse_escape( tok, escaped );
            if( multi && c==0 )
            {
                bool hit = false;
                while( is_whitespace( get_token( tok ) ) || parse_newline( tok ) )
                {
                    if( is_whitespace( get_token( tok ) ) )
                        parse_whitespace( tok );
                    if( parse_newline( tok ) )
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
                FAIL_BREAK( c<5, "parsed escape sequence is too long\n" )
                for( int i=0; i<c; i++ )
                    value[ idx++ ] = escaped[i];
                // parse_escape will parse everything and move on to the next token
                // so we call backtrack here to offset the next_token call outside
                backtrack( tok, 1 );
            }
        }
        else if( !multi && is_control( get_token( tok ) ) )
            LOG_ERR_BREAK( "control characters need to be escaped\n" )
        else if( multi && is_control_multi( get_token( tok ) ) )
            LOG_ERR_BREAK( "control characters need to be escaped\n" )
        else
            value[ idx++ ] = get_token( tok );
        next_token( tok );
    }
    return NULL;
}

char* parse_literalstring( tokenizer_t* tok, char* value, bool multi )
{
    size_t idx = 0;
    while( has_token( tok ) )
    {
        FAIL_BREAK( idx<MAX_STRING_LENGTH, "buffer overflow\n" );
        if( is_literalstringstart( get_token( tok ) ) )
        {
            if( !multi )
            {
                next_token( tok );
                return value;
            }
            else
            {
                int a = next_token( tok );
                int b = next_token( tok );
                if( is_literalstringstart( get_token( tok ) ) && is_literalstringstart( get_prev( tok ) ) )
                {
                    next_token( tok );
                    return value;
                }
                else
                {
                    value[ idx++ ] = '\'';
                    backtrack( tok, a+b-1 );
                    continue;
                }
            }
        }
        else if( parse_newline( tok ) && !multi )
            LOG_ERR_BREAK( "newline before end of string\n" )
        else if( parse_newline( tok ) && multi && idx==0 )
            ;
        else if( is_control_literal( get_token( tok ) ) )
            LOG_ERR_BREAK( "control characters need to be escaped\n" )
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
    while( has_token( tok ) )
    {
        FAIL_BREAK( idx<MAX_STRING_LENGTH, "buffer overflow\n" );
        if( ( is_whitespace( get_token( tok ) ) && spaces ) ||
            ( !is_whitespace( get_token( tok ) ) && is_numberend( get_token( tok ), num_end ) ) )
        {
            struct tm* time = calloc( 1, sizeof( struct tm ) );
            char year   [ 5 ] = { 0 };
            char mon    [ 3 ] = { 0 };
            char mday   [ 3 ] = { 0 };
            char hour   [ 3 ] = { 0 };
            char min    [ 3 ] = { 0 };
            char sec    [ 3 ] = { 0 };
            char delim  [ 2 ] = { 0 };
            char tz     [ 2 ] = { 0 };
            char off_s  [ 2 ] = { 0 };
            char off_h  [ 3 ] = { 0 };
            char off_m  [ 3 ] = { 0 };

            int t;
            t = sscanf( value, "%4c-%2c-%2c%1c%2c:%2c:%2c%1c%2c:%2c",
                        year, mon, mday, delim, hour, min, sec, off_s, off_h, off_m );
            if( t==10 )
            {
                FAIL_BREAK( strlen( year )==4, "invalid year\n" )
                FAIL_BREAK( strlen( mon )==2, "invalid month\n" )
                FAIL_BREAK( strlen( mday )==2, "invalid day\n" )
                FAIL_BREAK( strlen( delim )==1, "invalid delimiter\n" )
                FAIL_BREAK( strlen( hour )==2, "invalid hour\n" )
                FAIL_BREAK( strlen( min )==2, "invalid minute\n" )
                FAIL_BREAK( strlen( sec )==2, "invalid second\n" )
                FAIL_BREAK( strlen( off_s )==1, "invalid offset sign\n" )
                FAIL_BREAK( strlen( off_h )==2, "invalid offset hour\n" )
                FAIL_BREAK( strlen( off_m )==2, "invalid offset minute\n" )
                if( delim[ 0 ]==' ' )
                {
                    FAIL_BREAK( strlen( value )==strlen( "YYYY-mm-DD HH:MM:SS-HH:MM" ), "datetime has incorrect number of characters\n" )
                }
                else
                {
                    FAIL_BREAK( ( strlen( value )==( strlen( "YYYY-mm-DDTHH:MM:SS-HH:MM" )+spaces ) ), "datetime has incorrect number of characters\n" )
                }
                char* end;
                unsigned long num;
                num = strtoul( year, &end, 10 );
                FAIL_BREAK( end==year+strlen( year ), "invalid year\n" )
                time->tm_year = num-1900;
                num = strtoul( mon, &end, 10 );
                FAIL_BREAK( end==mon+strlen( mon ), "invalid month\n" )
                FAIL_BREAK( ( num>=1 && num<=12 ), "invalid month\n" )
                time->tm_mon = num-1;
                num = strtoul( mday, &end, 10 );
                FAIL_BREAK( end==mday+strlen( mday ), "invalid day\n" )
                FAIL_BREAK( is_date( time->tm_year, time->tm_mon, num ), "invalid day\n" )
                time->tm_mday = num;
                FAIL_BREAK( ( 0==strcmp( delim, "T" ) || 0==strcmp( delim, " " ) || 0==strcmp( delim, "t" ) ), "invalid delimiter\n" )
                num = strtoul( hour, &end, 10 );
                FAIL_BREAK( end==hour+strlen( hour ), "invalid hour\n" )
                FAIL_BREAK( ( num>=0 && num<=23 ), "invalid hour\n" )
                time->tm_hour = num;
                num = strtoul( min, &end, 10 );
                FAIL_BREAK( end==min+strlen( min ), "invalid minute\n" )
                FAIL_BREAK( ( num>=0 && num<=59 ), "invalid minute\n" )
                time->tm_min = num;
                num = strtoul( sec, &end, 10 );
                FAIL_BREAK( end==sec+strlen( sec ), "invalid seconds\n" )
                FAIL_BREAK( ( num>=0 && num<=59 ), "invalid seconds\n" )
                time->tm_sec = num;
                num = strtoul( off_h, &end, 10 );
                FAIL_BREAK( end==off_h+strlen( off_h ), "invalid offset hour\n" )
                FAIL_BREAK( ( num>=0 && num<=23 ), "invalid offset hour\n" )
                time->tm_gmtoff = num*60*60;
                num = strtoul( off_m, &end, 10 );
                FAIL_BREAK( end==off_m+strlen( off_m ), "invalid offset minute\n" )
                FAIL_BREAK( ( num>=0 && num<=59 ), "invalid offset minute\n" )
                time->tm_gmtoff += num;
                FAIL_BREAK( ( 0==strcmp( off_s, "+" ) || 0==strcmp( off_s, "-" ) ), "invalid offset sign\n" )
                if( 0==strcmp( off_s, "-" ) )
                    time->tm_gmtoff *= -1;
                dt = calloc( 1, sizeof( datetime_t ) );
                char* format = "%Y-%m-%dT%H:%M:%S-HH:MM";
                dt->format = calloc( 1, strlen( format )+1 );
                int c = snprintf( dt->format, strlen( format )+1, "%%Y-%%m-%%dT%%H:%%M:%%S%c%s:%s", off_s[ 0 ], off_h, off_m );
                dt->type = DATETIME;
                dt->dt = time;
                return dt;
            }
            t = sscanf( value, "%4c-%2c-%2c%1c%2c:%2c:%2c%1c",
                        year, mon, mday, delim, hour, min, sec, tz );
            if( t==8 )
            {
                FAIL_BREAK( strlen( year )==4, "invalid year\n" )
                FAIL_BREAK( strlen( mon )==2, "invalid month\n" )
                FAIL_BREAK( strlen( mday )==2, "invalid day\n" )
                FAIL_BREAK( strlen( delim )==1, "invalid delimiter\n" )
                FAIL_BREAK( strlen( hour )==2, "invalid hour\n" )
                FAIL_BREAK( strlen( min )==2, "invalid minute\n" )
                FAIL_BREAK( strlen( sec )==2, "invalid second\n" )
                FAIL_BREAK( strlen( tz )==1, "invalid timezone\n" )
                if( delim[ 0 ]==' ' )
                {
                    FAIL_BREAK( strlen( value )==strlen( "YYYY-mm-DD HH:MM:SSZ" ), "datetime has incorrect number of characters\n" )
                }
                else
                {
                    FAIL_BREAK( ( strlen( value )==( strlen( "YYYY-mm-DDTHH:MM:SSZ" )+spaces ) ), "datetime has incorrect number of characters\n" )
                }
                char* end;
                unsigned long num;
                num = strtoul( year, &end, 10 );
                FAIL_BREAK( end==year+strlen( year ), "invalid year\n" )
                time->tm_year = num-1900;
                num = strtoul( mon, &end, 10 );
                FAIL_BREAK( end==mon+strlen( mon ), "invalid month\n" )
                FAIL_BREAK( ( num>=1 && num<=12 ), "invalid month\n" )
                time->tm_mon = num-1;
                num = strtoul( mday, &end, 10 );
                FAIL_BREAK( end==mday+strlen( mday ), "invalid day\n" )
                FAIL_BREAK( is_date( time->tm_year, time->tm_mon, num ), "invalid day\n" )
                time->tm_mday = num;
                FAIL_BREAK( ( 0==strcmp( delim, "T" ) || 0==strcmp( delim, " " ) || 0==strcmp( delim, "t" ) ), "invalid delimiter\n" )
                num = strtoul( hour, &end, 10 );
                FAIL_BREAK( end==hour+strlen( hour ), "invalid hour\n" )
                FAIL_BREAK( ( num>=0 && num<=23 ), "invalid hour\n" )
                time->tm_hour = num;
                num = strtoul( min, &end, 10 );
                FAIL_BREAK( end==min+strlen( min ), "invalid minute\n" )
                FAIL_BREAK( ( num>=0 && num<=59 ), "invalid minute\n" )
                time->tm_min = num;
                num = strtoul( sec, &end, 10 );
                FAIL_BREAK( end==sec+strlen( sec ), "invalid seconds\n" )
                FAIL_BREAK( ( num>=0 && num<=59 ), "invalid seconds\n" )
                time->tm_sec = num;
                FAIL_BREAK( ( 0==strcmp( tz, "Z" ) || 0==strcmp( tz, "z" ) ), "invalid timezone\n" )
                time->tm_zone = "UTC";
                dt = calloc( 1, sizeof( datetime_t ) );
                dt->format = calloc( 1, strlen( "%Y-%m-%dT%H:%M:%S" ) );
                char* format = "%Y-%m-%dT%H:%M:%SZ";
                dt->format = calloc( 1, strlen( format )+1 );
                int c = snprintf( dt->format, strlen( format )+1, "%%Y-%%m-%%dT%%H:%%M:%%SZ" );
                dt->type = DATETIME;
                dt->dt = time;
                return dt;
            }
            t = sscanf( value, "%4c-%2c-%2c%1c%2c:%2c:%2c",
                        year, mon, mday, delim, hour, min, sec );
            if( t==7 )
            {
                FAIL_BREAK( strlen( year )==4, "invalid year\n" )
                FAIL_BREAK( strlen( mon )==2, "invalid month\n" )
                FAIL_BREAK( strlen( mday )==2, "invalid day\n" )
                FAIL_BREAK( strlen( delim )==1, "invalid delimiter\n" )
                FAIL_BREAK( strlen( hour )==2, "invalid hour\n" )
                FAIL_BREAK( strlen( min )==2, "invalid minute\n" )
                FAIL_BREAK( strlen( sec )==2, "invalid second\n" )
                if( delim[ 0 ]==' ' )
                {
                    FAIL_BREAK( strlen( value )==strlen( "YYYY-mm-DD HH:MM:SS" ), "datetime has incorrect number of characters\n" )
                }
                else
                {
                    FAIL_BREAK( ( strlen( value )==( strlen( "YYYY-mm-DDTHH:MM:SS" )+spaces ) ), "datetime has incorrect number of characters\n" )
                }
                char* end;
                unsigned long num;
                num = strtoul( year, &end, 10 );
                FAIL_BREAK( end==year+strlen( year ), "invalid year\n" )
                time->tm_year = num-1900;
                num = strtoul( mon, &end, 10 );
                FAIL_BREAK( end==mon+strlen( mon ), "invalid month\n" )
                FAIL_BREAK( ( num>=1 && num<=12 ), "invalid month\n" )
                time->tm_mon = num-1;
                num = strtoul( mday, &end, 10 );
                FAIL_BREAK( end==mday+strlen( mday ), "invalid day\n" )
                FAIL_BREAK( is_date( time->tm_year, time->tm_mon, num ), "invalid day\n" )
                time->tm_mday = num;
                FAIL_BREAK( ( 0==strcmp( delim, "T" ) || 0==strcmp( delim, " " ) || 0==strcmp( delim, "t" ) ), "invalid delimiter\n" )
                num = strtoul( hour, &end, 10 );
                FAIL_BREAK( end==hour+strlen( hour ), "invalid hour\n" )
                FAIL_BREAK( ( num>=0 && num<=23 ), "invalid hour\n" )
                time->tm_hour = num;
                num = strtoul( min, &end, 10 );
                FAIL_BREAK( end==min+strlen( min ), "invalid minute\n" )
                FAIL_BREAK( ( num>=0 && num<=59 ), "invalid minute\n" )
                time->tm_min = num;
                num = strtoul( sec, &end, 10 );
                FAIL_BREAK( end==sec+strlen( sec ), "invalid seconds\n" )
                FAIL_BREAK( ( num>=0 && num<=59 ), "invalid seconds\n" )
                time->tm_sec = num;
                dt = calloc( 1, sizeof( datetime_t ) );
                dt->format = calloc( 1, strlen( "%Y-%m-%dT%H:%M:%S" ) );
                memcpy( dt->format, "%Y-%m-%dT%H:%M:%S", strlen( "%Y-%m-%dT%H:%M:%S" ) );
                dt->type = DATETIMELOCAL;
                dt->dt = time;
                return dt;
            }
            t = sscanf( value, "%4c-%2c-%2c",
                        year, mon, mday );
            if( t==3 )
            {
                FAIL_BREAK( strlen( year )==4, "invalid year\n" )
                FAIL_BREAK( strlen( mon )==2, "invalid month\n" )
                FAIL_BREAK( strlen( mday )==2, "invalid day\n" )
                FAIL_BREAK( ( strlen( value )==( strlen( "YYYY-mm-DD" )+spaces ) ), "date has incorrect number of characters\n" )
                char* end;
                unsigned long num;
                num = strtoul( year, &end, 10 );
                FAIL_BREAK( end==year+strlen( year ), "invalid year\n" )
                time->tm_year = num-1900;
                num = strtoul( mon, &end, 10 );
                FAIL_BREAK( end==mon+strlen( mon ), "invalid month\n" )
                FAIL_BREAK( ( num>=1 && num<=12 ), "invalid month\n" )
                time->tm_mon = num-1;
                num = strtoul( mday, &end, 10 );
                FAIL_BREAK( end==mday+strlen( mday ), "invalid day\n" )
                FAIL_BREAK( is_date( time->tm_year, time->tm_mon, num ), "invalid day\n" )
                time->tm_mday = num;
                dt = calloc( 1, sizeof( datetime_t ) );
                dt->format = calloc( 1, strlen( "%Y-%m-%d" ) );
                memcpy( dt->format, "%Y-%m-%d", strlen( "%Y-%m-%d" ) );
                dt->type = DATELOCAL;
                dt->dt = time;
                return dt;
            }
            t = sscanf( value, "%2c:%2c:%2c",
                        hour, min, sec );
            if( t==3 )
            {
                FAIL_BREAK( strlen( hour )==2, "invalid hour\n" )
                FAIL_BREAK( strlen( min )==2, "invalid minute\n" )
                FAIL_BREAK( strlen( sec )==2, "invalid second\n" )
                FAIL_BREAK( ( strlen( value )==( strlen( "HH:MM:SS" )+spaces ) ), "time has incorrect number of characters\n" )
                char* end;
                unsigned long num;
                num = strtoul( hour, &end, 10 );
                FAIL_BREAK( end==hour+strlen( hour ), "invalid hour\n" )
                FAIL_BREAK( ( num>=0 && num<=23 ), "invalid hour\n" )
                time->tm_hour = num;
                num = strtoul( min, &end, 10 );
                FAIL_BREAK( end==min+strlen( min ), "invalid minute\n" )
                FAIL_BREAK( ( num>=0 && num<=59 ), "invalid minute\n" )
                time->tm_min = num;
                num = strtoul( sec, &end, 10 );
                FAIL_BREAK( end==sec+strlen( sec ), "invalid seconds\n" )
                FAIL_BREAK( ( num>=0 && num<=59 ), "invalid seconds\n" )
                time->tm_sec = num;
                dt = calloc( 1, sizeof( datetime_t ) );
                dt->format = calloc( 1, strlen( "%H:%M:%S" ) );
                memcpy( dt->format, "%H:%M:%S", strlen( "%H:%M:%S" ) );
                dt->type = TIMELOCAL;
                dt->dt = time;
                return dt;
            }
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
        else if( parse_newline( tok ) )
            next_token( tok );
        else if( is_whitespace( get_token( tok ) ) )
            parse_whitespace( tok );
        else if( is_commentstart( get_token( tok ) ) )
        {
            bool ok = parse_comment( tok );
            FAIL_BREAK( ok, "invalid comment\n" )
        }
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
    bool first = true;
    while( has_token( tok ) )
    {
        if( is_inlinetableend( get_token( tok ) ) )
        {
            FAIL_BREAK( ( !sep || first ), "cannot have trailing comma in inline table\n" )
            next_token( tok );
            return keys;
        }
        else if( is_inlinetablesep( get_token( tok ) ) )
        {
            FAIL_BREAK( !sep, "expected key-value but got , instead" )
            sep = true;
            next_token( tok );
        }
        else if( parse_newline( tok ) )
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
            // refer to inline table comment in `keys.c`
            if( v->type==INLINETABLE )
            {
                key_t* h = ( key_t * )( v->data );
                k->type = KEY;
                for( key_t** iter=h->subkeys; iter<h->last; iter++ )
                {
                    key_t* e = add_subkey( k, *iter );
                    FAIL_RETURN( e, "could not add inline table key %s\n", ( *iter )->id )
                }
                k->type = KEYLEAF;
            }
            else
                k->value = v;
            parse_whitespace( tok );
            sep = false;
            first = false;
        }
    }
    return NULL;
}

bool parse_comment( tokenizer_t* tok )
{
    while( has_token( tok ) )
    {
        next_token( tok );
        if( parse_newline( tok ) )
        {
            next_token( tok );
            return true;
        }
        if( is_control( get_token( tok ) ) )
            return false;
    }
    return true;
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

bool parse_newline( tokenizer_t* tok )
{
    if( is_newline( get_token( tok ) ) )
        return true;
    else if( is_return( get_token( tok ) ) )
    {
        bool a = next_token( tok );
        if( is_newline( get_token( tok ) ) )
            return true;
        else
            backtrack( tok, a );
    }
    return false;
}

int parse_unicode( tokenizer_t* tok, char* escaped )
{
    size_t digits = 0;
    char code[ 9 ] = { 0 };
    while( has_token( tok ) )
    {
        if( digits>8 )
            LOG_ERR_BREAK( "Invalid unicode escape code\n" )
        if( is_hexdigit( get_token( tok ) ) || is_digit( get_token( tok ) ) )
        {
            code[ digits++ ] = get_token( tok );
            next_token( tok );
            continue;
        }
        else
        {
            if( digits!=4 && digits!=8 )
                LOG_ERR_BREAK( "Invalid unicode escape code\n" )
            char* end;
            unsigned long num = strtoul( code, &end, 16 );
            if( end!=code+digits )
                LOG_ERR_BREAK( "Invalid unicode escape code\n" )
            // Unicode Scalar Values: %x80-D7FF / %xE000-10FFFF
            if( ( num>=0x0 && num<=0xD7FF ) || ( num>=0xE000 && num<=0x10FFFF ) )
            {
                // UTF-8 encoding
                if( num>=0x0 && num<=0x7F )
                {
                    escaped[0] = ( num ) & 0b01111111;
                    return 1;
                }
                else if( num>=0x80 && num<=0x7FF )
                {
                    escaped[0] = ( 0b11000000 | ( num >> 6 ) )  & 0b11011111;
                    escaped[1] = ( 0b10000000 | ( num ) )       & 0b10111111;
                    return 2;
                }
                else if( ( num>=0x800 && num<=0xFFFF ) )
                {
                    escaped[0] = ( 0b11100000 | ( num >> 12 ) ) & 0b11101111;
                    escaped[1] = ( 0b10000000 | ( num >> 6 ) )  & 0b10111111;
                    escaped[2] = ( 0b10000000 | ( num ) )       & 0b10111111;
                    return 3;
                }
                else
                {
                    escaped[0] = ( 0b11110000 | ( num >> 18 ) ) & 0b11110111;
                    escaped[1] = ( 0b10000000 | ( num >> 12 ) ) & 0b10111111;
                    escaped[2] = ( 0b10000000 | ( num >> 6 ) )  & 0b10111111;
                    escaped[3] = ( 0b10000000 | ( num ) )       & 0b10111111;
                    return 4;
                }
                return 0;
            }
            else
                LOG_ERR_BREAK( "Invalid unicode escape code\n" )
        }
    }
    return 0;
}

// TODO: unused
int parse_escape( tokenizer_t* tok, char* escaped )
{
    switch ( get_token( tok ) )
    {
        case 'b':
        {
            escaped[0] = '\b';
            next_token( tok );
            return 1;
        }
        case 't':
        {
            escaped[0] = '\t';
            next_token( tok );
            return 1;
        }
        case 'n':
        {
            escaped[0] = '\n';
            next_token( tok );
            return 1;
        }
        case 'f':
        {
            escaped[0] = '\f';
            next_token( tok );
            return 1;
        }
        case 'r':
        {
            escaped[0] = '\r';
            next_token( tok );
            return 1;
        }
        case '"':
        {
            escaped[0] = '\"';
            next_token( tok );
            return 1;
        }
        case '\\':
        {
            escaped[0] = '\\';
            next_token( tok );
            return 1;
        }
        case 'u':
        {
            next_token( tok );
            int u = parse_unicode( tok, escaped );
            return u;
        }
        case 'U':
        {
            next_token( tok );
            int u = parse_unicode( tok, escaped );
            return u;
        }
        default:
            return 0;
    }
    return 0;
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
        FAIL_BREAK( !parse_newline( tok ), "got a newline before any value\n" )
        if( is_whitespace( get_token( tok ) ) )
        {
            parse_whitespace( tok );
            continue;
        }
        else if( is_basicstringstart( get_token( tok ) ) )
        {
            char value[ MAX_STRING_LENGTH ] = { 0 };
            char* s;
            next_token( tok );
            if( has_token( tok ) && is_basicstringstart( get_token( tok ) ) )
            {
                next_token( tok );
                if( has_token( tok ) && is_basicstringstart( get_token( tok ) ) )
                {
                    next_token( tok );
                    s = parse_basicstring( tok, value, true );
                }
                else if( is_whitespace( get_token( tok ) ) || parse_newline( tok ) )
                    s = value;
                else
                    LOG_ERR_BREAK( "cannot start string with 2 double-quotes\n" )
            }
            else
                s = parse_basicstring( tok, value, false );
            FAIL_BREAK( s, "could not parse basic string\n" )
            value_t* v = new_string( value );
            return v;
        }
        else if( is_literalstringstart( get_token( tok ) ) )
        {
            char value[ MAX_STRING_LENGTH ] = { 0 };
            char* s;
            next_token( tok );
            if( has_token( tok ) && is_literalstringstart( get_token( tok ) ) )
            {
                next_token( tok );
                if( has_token( tok ) && is_literalstringstart( get_token( tok ) ) )
                {
                    next_token( tok );
                    s = parse_literalstring( tok, value, true );
                }
                else if( is_whitespace( get_token( tok ) ) || parse_newline( tok ) )
                    s = value;
                else
                    LOG_ERR_BREAK( "cannot start string with 2 single-quotes\n" )
            }
            else
                s = parse_literalstring( tok, value, false );
            FAIL_BREAK( s, "could not parse literal string\n" )
            value_t* v = new_string( value );
            return v;
        }
        else if( is_numberstart( get_token( tok ) ) )
        {
            char value[ MAX_STRING_LENGTH ] = { 0 };
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

#include "utils.h"

#include <string.h>

bool is_whitespace( char c )
{
    return ( c==' ' || c=='\t' );
}

bool is_newline( char c )
{
    return ( c=='\n' );
}

bool is_return( char c )
{
    return ( c=='\r' );
}

bool is_commentstart( char c )
{
    return ( c=='#' );
}

bool is_equal( char c )
{
    return ( c=='=' );
}

bool is_escape( char c )
{
    return ( c=='\\' );
}

bool is_basicstringstart( char c )
{
    return ( c=='"' );
}

bool is_literalstringstart( char c )
{
    return ( c=='\'' );
}

bool is_tablestart( char c )
{
    return ( c=='[' );
}

bool is_tableend( char c )
{
    return ( c==']' );
}

bool is_inlinetablestart( char c )
{
    return ( c=='{' );
}

bool is_inlinetableend( char c )
{
    return ( c=='}' );
}

bool is_inlinetablesep( char c )
{
    return ( c==',' );
}

bool is_dot( char c )
{
    return ( c=='.' );
}

bool is_digit( char c )
{
    return ( c>='0' && c<='9' );
}

bool is_hexdigit( char c )
{
    return (
        ( c>='A' && c<='F') ||
        ( c>='a' && c<='f')
    );
}

bool is_numberstart( char c )
{
    return (
        ( c=='+' || c=='-') ||
        is_digit( c )
    );
}

bool is_bare_ascii( char c )
{
    return (
        ( c>='A' && c<='Z') ||
        ( c>='a' && c<='z') ||
        ( c=='_' || c=='-') ||
        is_digit( c )
    );
}

bool is_numberend( char c, const char* end )
{
    for( size_t i=0; i<strlen( end ); i++ )
    {
        if ( c==end[ i ] )
            return true;
    }
    return false;
}

bool is_decimalpoint( char c )
{
    return ( c=='.' );
}

bool is_underscore( char c )
{
    return ( c=='_' );
}

bool is_arraystart( char c )
{
    return ( c=='[' );
}

bool is_arrayend( char c )
{
    return ( c==']' );
}

bool is_arraysep( char c )
{
    return ( c==',' );
}

#include "parse_utils.h"

#include <string.h>

bool
is_whitespace( char c ) {
    return ( c==' ' || c=='\t' );
}

bool
is_newline( char c ) {
    return ( c=='\n' );
}

bool
is_return( char c ) {
    return ( c=='\r' );
}

bool
is_commentstart( char c ) {
    return ( c=='#' );
}

bool
is_equal( char c ) {
    return ( c=='=' );
}

bool
is_escape( char c ) {
    return ( c=='\\' );
}

bool
is_basicstringstart( char c ) {
    return ( c=='"' );
}

bool
is_literalstringstart( char c ) {
    return ( c=='\'' );
}

bool
is_tablestart( char c ) {
    return ( c=='[' );
}

bool
is_tableend( char c ) {
    return ( c==']' );
}

bool
is_inlinetablestart( char c ) {
    return ( c=='{' );
}

bool
is_inlinetableend( char c ) {
    return ( c=='}' );
}

bool
is_inlinetablesep( char c ) {
    return ( c==',' );
}

bool
is_dot( char c ) {
    return ( c=='.' );
}

bool
is_digit( char c ) {
    return ( c>='0' && c<='9' );
}

bool
is_hexdigit( char c ) {
    return (
        ( c>='A' && c<='F') ||
        ( c>='a' && c<='f')
    );
}

bool
is_numberstart( char c ) {
    return (
        ( c=='+' || c=='-') ||
        is_digit( c )
    );
}

bool
is_bare_ascii( char c ) {
    return (
        ( c>='A' && c<='Z') ||
        ( c>='a' && c<='z') ||
        ( c=='_' || c=='-') ||
        is_digit( c )
    );
}

bool
is_control( char c ) {
    return (
        ( c>=0x0 && c<=0x8 )  ||
        ( c>=0xA && c<=0x1F ) ||
        ( c==0x7F )
    );
}

bool
is_control_multi( char c ) {
    return (
        ( c>=0x0 && c<=0x8 )  ||
        ( c==0xB || c==0xC )  ||
        ( c>=0xE && c<=0x1F ) ||
        ( c==0x7F )
    );
}

bool
is_control_literal( char c ) {
    return (
        ( c!=0x9 ) &&
        ( c!=0xA ) &&
        ( c>=0x0 && c<=0x1F ) ||
        ( c==0x7F )
    );
}

bool
is_numberend(
    char c,
    const char* end
) {
    for( int i=0; i<strlen( end ); i++ )
    {
        if ( c==end[ i ] ) return true;
    }
    return false;
}

bool
is_decimalpoint( char c ) {
    return ( c=='.' );
}

bool
is_underscore( char c ) {
    return ( c=='_' );
}

bool
is_arraystart( char c ) {
    return ( c=='[' );
}

bool
is_arrayend( char c ) {
    return ( c==']' );
}

bool
is_arraysep( char c ) {
    return ( c==',' );
}

bool
is_date(
    int year,
    int month,
    int day
) {
    switch ( month ) {
        // January
        case 0:
            return ( day>=1 && day<=31 );
        // February
        case 1:
            return ( ( day>=1 && day<=28 ) ||
                     ( day==29 && year%4==0 && year%100!=0 ) ||
                     ( day==29 && year%4==0 && year%100==0 && year%400==0 )
            );
        // March
        case 2:
            return ( day>=1 && day<=31 );
        // April
        case 3:
            return ( day>=1 && day<=30 );
        // May
        case 4:
            return ( day>=1 && day<=31 );
        // June
        case 5:
            return ( day>=1 && day<=30 );
        // July
        case 6:
            return ( day>=1 && day<=31 );
        // August
        case 7:
            return ( day>=1 && day<=31 );
        // September
        case 8:
            return ( day>=1 && day<=30 );
        // October
        case 9:
            return ( day>=1 && day<=31 );
        // November
        case 10:
            return ( day>=1 && day<=30 );
        // December
        case 11:
            return ( day>=1 && day<=31 );
        default:
            return false;
    }
    return false;
}

bool
is_validdatetime( struct tm* datetime ) {
    return( ( datetime->tm_hour>=0 && datetime->tm_hour<=23 ) &&
            ( datetime->tm_min>=0 && datetime->tm_min<=59 ) &&
            ( datetime->tm_sec>=0 && datetime->tm_sec<=59 ) &&
            is_date( datetime->tm_year+1900, datetime->tm_mon, datetime->tm_mday )
    );
}

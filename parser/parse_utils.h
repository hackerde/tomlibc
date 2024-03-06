#ifndef __TOMLIBC_PARSE_UTILS_H__
#define __TOMLIBC_PARSE_UTILS_H__

#include <stdbool.h>

/*
    All of these are utility functions to
    check character criteria. Most of these
    are defined according to TOML spec.
    Some of these check the same thing, but
    they are defined multitple times for code
    readability.
*/
bool is_whitespace( char c );
bool is_newline( char c );
bool is_return( char c );
bool is_commentstart( char c );
bool is_equal( char c );
bool is_escape( char c );
bool is_basicstringstart( char c );
bool is_literalstringstart( char c );
bool is_tablestart( char c );
bool is_tableend( char c );
bool is_inlinetablestart( char c );
bool is_inlinetableend( char c );
bool is_inlinetablesep( char c );
bool is_dot( char c );
bool is_digit( char c );
bool is_hexdigit( char c );
bool is_numberstart( char c );
bool is_bare_ascii( char c );
bool is_control( char c );
bool is_control_multi( char c );
bool is_control_literal( char c );
bool is_numberend( char c, const char* end );
bool is_decimalpoint( char c );
bool is_underscore( char c );
bool is_arraystart( char c );
bool is_arrayend( char c );
bool is_arraysep( char c );
bool is_date( int year, int month, int day );

#endif

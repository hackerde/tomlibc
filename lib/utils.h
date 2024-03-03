#ifndef __LIB_UTILS_H__
#define __LIB_UTILS_H__

#include <stdio.h>

/*
    Macro `LOG_ERR` is for logging an error
    message to stderr, that also specifies
    which file, line and function the error
    was raised in.
*/
#define LOG_ERR( ... )                      \
    { fprintf( stderr, "%s:%d [%s]: ",      \
    __FILE__, __LINE__, __func__ );         \
    fprintf( stderr, __VA_ARGS__ ); }

/*
    Macro `LOG_ERR_BREAK` calls `LOG_ERR`
    and then calls a `break;`
*/
#define LOG_ERR_BREAK( ... )                \
    { LOG_ERR( __VA_ARGS__ );               \
    break; }

/*
    Macro `LOG_ERR_RETURN` calls `LOG_ERR`
    and then returns NULL.
*/
#define LOG_ERR_RETURN( ... )               \
    { LOG_ERR( __VA_ARGS__ );               \
    return NULL; }

/*
    Macro `FAIL_BREAK` calls `LOG_ERR_BREAK`
    if the given condition fails.
*/
#define FAIL_BREAK( COND, ... )             \
    if( !( COND ) )                         \
        LOG_ERR_BREAK( __VA_ARGS__ );       \

/*
    Macro `FAIL_RETURN` calls `LOG_ERR_RETURN`
    if the given condition fails.
*/
#define FAIL_RETURN( COND, ... )            \
    if( !( COND ) )                         \
        LOG_ERR_RETURN( __VA_ARGS__ );      \

#endif

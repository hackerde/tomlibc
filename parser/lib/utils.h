#ifndef __TOMLIBC_UTILS_H__
#define __TOMLIBC_UTILS_H__

#include <stdio.h>

/*
    Macro `LOG_ERR` is for logging an error
    message to stderr, that also specifies
    which file, line and function the error
    was raised in.
*/
#define LOG_ERR( ... )                      \
    do                                      \
    {                                       \
        fprintf( stderr, "%s:%d [%s]: ",    \
        __FILE__, __LINE__, __func__ );     \
        fprintf( stderr, __VA_ARGS__ );     \
    } while( 0 )

/*
    Macro `RETURN_ON_FAIL` checks `COND` and
    if it fails, returns NULL from the location
    where it is called.
*/
#define RETURN_ON_FAIL( COND, ... )         \
    do                                      \
    {                                       \
        if( !( COND ) )                     \
        {                                   \
            LOG_ERR( __VA_ARGS__ );         \
            return NULL;                    \
        }                                   \
    } while( 0 )

#endif

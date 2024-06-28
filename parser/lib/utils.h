#ifndef __TOMLIBC_UTILS_H__
#define __TOMLIBC_UTILS_H__

#include <stdio.h>

/*
    Macro `LOG_ERR` is for logging an error
    message to stderr, that also specifies
    which file, line and function the error
    was raised in.
*/
#define LOG_ERR( ... )                   \
    do {                                 \
        fprintf( stderr, "%s:%d [%s]: ", \
        __FILE__, __LINE__, __func__ );  \
        fprintf( stderr, __VA_ARGS__ );  \
    } while( 0 )

/*
    Macro `RETURN_IF_FAILED` checks `COND` and
    if it fails, returns NULL from the location
    where it is called.
*/
#define RETURN_IF_FAILED( COND, ... ) \
    do {                              \
        if( !( COND ) ) {             \
            LOG_ERR( __VA_ARGS__ );   \
            return NULL;              \
        }                             \
    } while( 0 )

/*
    Macro `FUNC_IF_FAILED` checks `COND` and
    if it fails, calls FUNC with the args
    passed.
*/
#define FUNC_IF_FAILED( COND, FUNC, ... ) \
    do {                                  \
        if( !( COND ) ) {                 \
            FUNC( __VA_ARGS__ );          \
        }                                 \
    } while( 0 )

#endif

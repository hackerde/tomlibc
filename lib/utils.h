#ifndef __LIB_UTILS_H__
#define __LIB_UTILS_H__

#include <stdio.h>

#define LOG_ERR( ... )                      \
    { fprintf( stderr, "%s:%d [%s]: ",      \
    __FILE__, __LINE__, __func__ );         \
    fprintf( stderr, __VA_ARGS__ ); }

#define LOG_ERR_BREAK( ... )                \
    { LOG_ERR( __VA_ARGS__ );               \
    break; }

#define LOG_ERR_RETURN( ... )               \
    { LOG_ERR( __VA_ARGS__ );               \
    return NULL; }

#define FAIL_BREAK( COND, ... )             \
    if( !( COND ) )                         \
        LOG_ERR_BREAK( __VA_ARGS__ );       \

#define FAIL_RETURN( COND, ... )            \
    if( !( COND ) )                         \
        LOG_ERR_RETURN( __VA_ARGS__ );      \

#endif
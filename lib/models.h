#ifndef __LIB_MODELS_H__
#define __LIB_MODELS_H__

#include <stdio.h>

#define bool    unsigned char
#define uchar   unsigned char
#define true    1
#define false   0

#define MAX_ID_LENGTH       512
#define MAX_NUM_SUBKEYS     1024
#define MAX_STRING_LENGTH   512
#define MAX_ARRAY_LENGTH    50

typedef enum value_type value_type_t;
enum value_type
{
    STRING,
    INT,
    FLOAT,
    BOOL,
    DATETIME,
    DATETIMELOCAL,
    DATELOCAL,
    TIMELOCAL,
    ARRAY,
    INLINETABLE,
    GARBAGE,
};

typedef struct value value_t;
struct value
{
    value_type_t    type;
    value_t**       arr;
    void*           data;
    size_t          precision;
    bool            scientific;
    char*           format;
};

typedef enum key_type key_type_t;
enum key_type
{
    TABLE,
    KEY,
    ARRAYTABLE,
    TABLELEAF,
    KEYLEAF,
};

typedef struct key key_t;
struct key
{
    key_type_t  type;
    char        id[ MAX_ID_LENGTH ];
    key_t*      subkeys[ MAX_NUM_SUBKEYS ];
    key_t**     last;
    value_t*    value;
    size_t      idx;
};

#endif

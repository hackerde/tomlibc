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

/*
    Enum `value_type` represents the set of value types
    accepted by this TOML parser. This corresponds to
    the various types defined in the TOML Spec.
*/
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
};

/*
    Struct `value` holds the various attributes associated
    with a TOML value.
*/
typedef struct value value_t;
struct value
{
    /* the type of TOML value */
    value_type_t    type;
    /* used for storing `ARRAY` type values */
    value_t**       arr;
    /* used for storing non-`ARRAY` type values */
    void*           data;
    /* used for printing numeric values */
    size_t          precision;
    bool            scientific;
    /* used for printing datetime values */
    char*           format;
};

/*
    Enum `key_type` represents the various types of
    keys that this TOML parser is aware of. It is used
    to make re-defining validity decisions.
*/
typedef enum key_type key_type_t;
enum key_type
{
    /* `[a.b]` -> a */
    TABLE,
    /* `j.k = v` -> j */
    KEY,
    /* `[[t]]` -> t */
    ARRAYTABLE,
    /* `[a.b]` -> b */
    TABLELEAF,
    /* `j.k = v` -> k */
    KEYLEAF,
};

/*
    Struct `key` defines the TOML keys. Each node in the
    parsed AST is a `key`, irrespective of the fact if
    they were defined as TOML keys or tables.
*/
typedef struct key key_t;
struct key
{
    /* key type as described above */
    key_type_t  type;
    /* identifier */
    char        id[ MAX_ID_LENGTH ];
    /* list of subkeys */
    key_t*      subkeys[ MAX_NUM_SUBKEYS ];
    /* pointer to the last added subkey */
    key_t**     last;
    /* value associated with this key */
    value_t*    value;
    /* used for indexing ARRAYTABLES */
    size_t      idx;
};

#endif

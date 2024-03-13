#ifndef __TOMLIBC_MODELS_H__
#define __TOMLIBC_MODELS_H__

#include "khash.h"

#include <stdbool.h>

#define TOML_MAX_DATE_FORMAT    64
#define TOML_MAX_ID_LENGTH      256
#define TOML_MAX_STRING_LENGTH  4096

#define TOML_MAX_SUBKEYS        131072  // 2^17
#define TOML_MAX_ARRAY_LENGTH   131072  // 2^17

/*
    Enum `toml_value_type` represents the set of value
    types accepted by this TOML parser. This corresponds
    to the various types defined in the TOML Spec.
*/
typedef enum toml_value_type toml_value_type_t;
enum toml_value_type
{
    TOML_STRING,
    TOML_INT,
    TOML_FLOAT,
    TOML_BOOL,
    TOML_DATETIME,
    TOML_DATETIMELOCAL,
    TOML_DATELOCAL,
    TOML_TIMELOCAL,
    TOML_ARRAY,
    TOML_INLINETABLE,
};

/*
    Struct `toml_value` holds the various attributes
    associated with a TOML value.
*/
typedef struct toml_value toml_value_t;
struct toml_value
{
    /* the type of TOML value */
    toml_value_type_t   type;
    /* used for storing `ARRAY` type values */
    toml_value_t**      arr;
    int                 len;
    /* used for storing non-`ARRAY` type values */
    void*               data;
    /* used for printing numeric values */
    int                 precision;
    bool                scientific;
    /* used for printing datetime values */
    char                format[ TOML_MAX_DATE_FORMAT ];
};

/*
    Enum `toml_key_type` represents the various types of
    keys that this TOML parser is aware of. It is used
    to make re-defining validity decisions.
*/
typedef enum toml_key_type toml_key_type_t;
enum toml_key_type
{
    /* `[a.b]` -> a */
    TOML_TABLE,
    /* `j.k = v` -> j */
    TOML_KEY,
    /* `[[t]]` -> t */
    TOML_ARRAYTABLE,
    /* `[a.b]` -> b */
    TOML_TABLELEAF,
    /* `j.k = v` -> k */
    TOML_KEYLEAF,
};

/*
    Struct `toml_key` defines the TOML keys. Each node in
    the parsed AST is a `key`, irrespective of the fact
    if they were defined as TOML keys or tables.
*/
typedef struct toml_key toml_key_t;
KHASH_MAP_INIT_STR( str, toml_key_t* )
struct toml_key
{
    /* key type as described above */
    toml_key_type_t type;
    /* identifier */
    char            id[ TOML_MAX_ID_LENGTH ];
    /* map of subkeys */
    khash_t( str )* subkeys;
    /* value associated with this key */
    toml_value_t*   value;
    /* used for indexing ARRAYTABLES */
    int             idx;
};

#endif

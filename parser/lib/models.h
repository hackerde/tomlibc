#ifndef __TOMLIBC_MODELS_H__
#define __TOMLIBC_MODELS_H__

#include <stdbool.h>

#define MAX_ID_LENGTH       512
#define MAX_NUM_SUBKEYS     1024
#define MAX_STRING_LENGTH   512
#define MAX_ARRAY_LENGTH    50

/*
    Enum `toml_value_type` represents the set of value
    types accepted by this TOML parser. This corresponds
    to the various types defined in the TOML Spec.
*/
typedef enum toml_value_type toml_value_type_t;
enum toml_value_type
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
    /* used for storing non-`ARRAY` type values */
    void*               data;
    /* used for printing numeric values */
    int                 precision;
    bool                scientific;
    /* used for printing datetime values */
    char*               format;
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
    Struct `toml_key` defines the TOML keys. Each node in
    the parsed AST is a `key`, irrespective of the fact
    if they were defined as TOML keys or tables.
*/
typedef struct toml_key toml_key_t;
struct toml_key
{
    /* key type as described above */
    toml_key_type_t type;
    /* identifier */
    char            id[ MAX_ID_LENGTH ];
    /* list of subkeys */
    toml_key_t*     subkeys[ MAX_NUM_SUBKEYS ];
    /* pointer to the last added subkey */
    toml_key_t**    last;
    /* value associated with this key */
    toml_value_t*   value;
    /* used for indexing ARRAYTABLES */
    int             idx;
};

#endif

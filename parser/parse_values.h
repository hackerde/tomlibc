#ifndef __TOMLIBC_PARSE_VALUES_H__
#define __TOMLIBC_PARSE_VALUES_H__

#include "lib/tokenizer.h"

/*
    Struct `number` creates a generic type
    for holding a parsed FLOAT and INT type
    numbers. It also holds precision for FLOATS
    which is mostly needed for compliance testing.
*/
typedef struct number number_t;
struct number
{
    toml_value_type_t   type;
    int                 precision;
    bool                scientific;
};

/*
    Struct `datetime` creates a generic type
    for holding parsed DATETIME, DATETIMELOCAL,
    DATELOCAL and TIMELOCAL values. It also stores
    the matching format, again for compliance testing.
*/
typedef struct datetime datetime_t;
struct datetime
{
    struct tm*          dt;
    toml_value_type_t   type;
    char*               format;
    int                 millis;
};

/*
    Functions `parse_<TYPE>` parses a TOML value of type
    TYPE. They take the tokenizer and parses one character
    at a time. Numerical values and datetimes have a list
    of characters to mark the end of parsing. Strings take
    in pre-allocated buffers. Arrays repeatedly parse values.
    Inline tables repeatedly parse key-value pairs. Everything
    returns a pointer to what it parsed and NULL on parsing
    failure. `parse_comment` returns true if a valid comment
    was parsed and `parse_newline` returns true if a newline
    was successfully parsed.
*/
bool        parse_comment       ( tokenizer_t* tok );
void        parse_whitespace    ( tokenizer_t* tok );
bool        parse_newline       ( tokenizer_t* tok );
double      parse_boolean       ( tokenizer_t* tok );
toml_key_t* parse_inlinetable   ( tokenizer_t* tok );

int parse_escape(
    tokenizer_t*    tok,
    char*           escaped
);

int parse_unicode(
    tokenizer_t*    tok,
    char*           escaped
);

char* parse_basicstring(
    tokenizer_t*    tok,
    char*           value,
    bool            multi
);

char* parse_literalstring(
    tokenizer_t*    tok,
    char*           value,
    bool            multi
);

double parse_inf_nan(
    tokenizer_t*    tok,
    bool            negative
);

double parse_base_uint(
    tokenizer_t*    tok,
    int             base,
    char*           value,
    const char*     num_end
);

number_t* parse_number(
    tokenizer_t*    tok,
    char*           value,
    double*         d,
    const char*     num_end
);

datetime_t* parse_datetime(
    tokenizer_t*    tok,
    char*           value,
    const char*     num_end
);

toml_value_t* parse_array(
    tokenizer_t*    tok,
    toml_value_t*   arr
);

/*
    Function `parse_value` looks at a character
    and decides what `TYPE` it is. Depending on that,
    it calls the appropriate `parse_<TYPE>` function.
    Since the `num_end` character set changes based
    on parsing context (for example, a number can end
    when we see a `,` when parsing an array), this
    is added as an argument to this function. This
    allows it to be used anywhere a value needs to be
    parsed.
*/
toml_value_t* parse_value(
    tokenizer_t*    tok,
    const char*     num_end
);

#endif

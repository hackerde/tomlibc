#ifndef __SRC_VALUES_H__
#define __SRC_VALUES_H__

#include "../lib/tokenizer.h"

typedef struct number number_t;
struct number
{
    value_type_t    type;
    size_t          precision;
    bool            scientific;
};

typedef struct datetime datetime_t;
struct datetime
{
    struct tm*      dt;
    value_type_t    type;
    char*           format;
};

char*       parse_basicstring   ( tokenizer_t* tok, char* value );
char*       parse_literalstring ( tokenizer_t* tok, char* value );
double      parse_inf_nan       ( tokenizer_t* tok, bool negative );
value_t*    parse_array         ( tokenizer_t* tok, value_t* arr );
double      parse_boolean       ( tokenizer_t* tok );
key_t*      parse_inlinetable   ( tokenizer_t* tok );
void        parse_comment       ( tokenizer_t* tok );
void        parse_whitespace    ( tokenizer_t* tok );
char*       parse_escape        ( tokenizer_t* tok );

datetime_t* parse_datetime(
    tokenizer_t*    tok,
    char*           value,
    const char*     num_end
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

value_t* parse_value(
    tokenizer_t*    tok,
    const char*     num_end
);

#endif

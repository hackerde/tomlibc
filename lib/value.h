#ifndef __LIB_VALUE_H__
#define __LIB_VALUE_H__

#include "models.h"

#include <time.h>

void        delete_value        ( value_t* v );

value_t*    new_array           ( );
value_t*    new_inline_table    ( key_t* k );
value_t*    new_string          ( const char* s );

value_t*    new_datetime(
    struct tm*      dt,
    value_type_t    type,
    char*           format
);

value_t*    new_number(
    double*         d,
    value_type_t    type,
    size_t          precision,
    bool            scientific
);

#endif

#ifndef __LIB_VALUE_H__
#define __LIB_VALUE_H__

#include "models.h"

#include <time.h>

/*
    Function `delete_value` frees up all the memory
    that is associated with value `v`.
*/
void        delete_value        ( value_t* v );

/*
    Function `new_array` allocates a buffer for an
    array of `value_t` and returns a pointer to it.
    This is stored in the `arr` attribute.
*/
value_t*    new_array           ( );

/*
    Function `new_inline_table` takes a key `k` as
    it's argument which can contain one or many key
    value pairs, including subkeys. It then allocates
    some new memory for a key and adds all subkeys of
    `k` as subkeys of this newly allocated key. It
    returns a pointer to the newly allocated value.
    The `data` attribute contains the newly created key.
*/
value_t*    new_inline_table    ( key_t* k );

/*
    Functions `new_string`, `new_datetime` and `new_number`
    allocates some memory for each of these datatypes
    respectively. It then copies over the passed in data
    into the `data` attribute of the value. Finally, like
    the other functions, it returns a pointer to the newly
    allocated value.
*/
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

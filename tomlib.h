#ifndef __TOMLIB_H__
#define __TOMLIB_H__

#include "parser/lib/models.h"

/*
    Function `toml_load` loads a TOML from either
    a file or from stdin if `file` is NULL. It
    initializes the tokenizer, and reads the input
    stream until we reach an EOF. Upon success, it
    returns a pointer to the `root` key and NULL on
    failure.
*/
toml_key_t* toml_load   ( const char* file );

/*
    Function `dump_key`, `dump_value` and
    `dump_toml` are functions to print out the
    TOML data in a JSON format for compliance testing.
*/
void toml_key_dump      ( toml_key_t* k );
void toml_value_dump    ( toml_value_t* v );

void toml_json_dump     ( toml_key_t* root );

void toml_free          ( toml_key_t* root );

/*
    ==CALLBACKS==
*/
/*
    Function `get_key` tries to return a key based on
    the argument `id`. If the existing `key->id` is not
    a match, it iterates through the list of `children`.
    It returns a pointer to the first match; if nothing
    matches, it returns NULL.
*/
toml_key_t* toml_get_key(
    toml_key_t* key,
    const char* id
);

#endif

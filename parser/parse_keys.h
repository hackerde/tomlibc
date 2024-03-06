#ifndef __TOMLIBC_PARSE_KEYS_H__
#define __TOMLIBC_PARSE_KEYS_H__

#include "lib/tokenizer.h"

/*
    Functions `parse_barekey`, `parse_basicquotedkey`
    and `parse_literalquotedkey` parses a "bare key",
    a "basic quoted key" and a "literal quoted key"
    respectively, as defined by the TOML spec. The caller
    should decide which one is being parsed. They use `.`
    and `=` as delimiters. Once it has successfully parsed
    a key, it creates a key and returns a pointer to it.
    All of them log errors and return NULL on parsing
    failure. The key types `branch` and `leaf` passed as
    arguments determine the key types of keys created upon
    encountering a `.` and a `=` respectively. The `end`
    argument determines which character marks the termination
    of parsing.
*/
toml_key_t* parse_barekey(
    tokenizer_t*    tok,
    char            end,
    toml_key_type_t branch,
    toml_key_type_t leaf
);

toml_key_t* parse_basicquotedkey(
    tokenizer_t*    tok,
    char            end,
    toml_key_type_t branch,
    toml_key_type_t leaf
);

toml_key_t* parse_literalquotedkey(
    tokenizer_t*    tok,
    char            end,
    toml_key_type_t branch,
    toml_key_type_t leaf
);

/*
    Functions `parse_key`, `parse_table` and
    `parse_arraytable` tries to parse a TOML
    key, table and arraytable respectively. It
    takes a `key` as its argument meaning the
    new key(s) that is(are) parsed will be added
    to the `subkeys` of `key`. Setting `expecting`
    to `true` will throw an error if we encounter
    a delimiter like `.` or `=`. Returns NULL on
    parsing failure.
*/
toml_key_t* parse_key(
    tokenizer_t*    tok,
    toml_key_t*     key,
    bool            expecting
);

toml_key_t* parse_table(
    tokenizer_t*    tok,
    toml_key_t*     key,
    bool            expecting
);

toml_key_t* parse_arraytable(
    tokenizer_t*    tok,
    toml_key_t*     key,
    bool            expecting
);

/*
    Function `parse_keyval` tries to parse a key
    and a value as defined by TOML. This is the entry
    point to parsing the TOML file. It takes a `key`
    and adds all the newly parsed keys as its `subkeys`.
    Since we end up creating a tree structure with the
    keys, we use a `root` key to refer to the whole
    tree. All keys are descendents of this key and this
    key refers to the entire TOML file. This function
    returns NULL on failure to parse. It returns a
    pointer to the last table or arraytable it parsed
    and returns `key` if neither.
*/
toml_key_t* parse_keyval(
    tokenizer_t*    tok,
    toml_key_t*     key,
    toml_key_t*     root
);

#endif

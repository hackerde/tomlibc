#ifndef __SRC_KEYS_H__
#define __SRC_KEYS_H__

#include "../lib/tokenizer.h"

key_t* parse_barekey(
    tokenizer_t*    tok,
    char            end,
    key_type_t      branch,
    key_type_t      leaf
);

key_t* parse_basicquotedkey(
    tokenizer_t*    tok,
    char            end,
    key_type_t      branch,
    key_type_t      leaf
);

key_t* parse_literalquotedkey(
    tokenizer_t*    tok,
    char            end,
    key_type_t      branch,
    key_type_t      leaf
);

key_t* parse_key(
    tokenizer_t*    tok,
    key_t*          key,
    bool            expecting
);

key_t* parse_table(
    tokenizer_t*    tok,
    key_t*          key,
    bool            expecting
);

key_t* parse_arraytable(
    tokenizer_t*    tok,
    key_t*          key,
    bool            expecting
);

key_t* parse_keyval(
    tokenizer_t*    tok,
    key_t*          key,
    key_t*          root
);

#endif

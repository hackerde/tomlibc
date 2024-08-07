#ifndef __TOMLIBC_KEY_H__
#define __TOMLIBC_KEY_H__

#include "models.h"

/*
    Function `new_key` allocates memory to create
    a new key/node in the AST. It takes the key type
    as an argument and initializes everything else
    to NULL and idx to 0. Returns a pointer to the
    newly allocated key.
*/
toml_key_t*
new_key( toml_key_type_t type );

/*
    Function `delete_key` frees up all the memory allocated
    by this key. It first recursively frees up all the
    memory allocated by the keys in `children` if any. Then
    it frees up all the memory allocated by `value` if any.
    Finally, it frees up the memory allocated by itself.
*/
void
delete_key( toml_key_t* key );

/*
    Function `has_subkey` checks if a `key` has a `subkey`
    in its list of `children` by iterating through it
    and matching against the `id`. Returns a pointer
    to the key if it exists, else returns NULL.
*/
toml_key_t* 
has_subkey(
    toml_key_t* key,
    toml_key_t* subkey
);

/*
    Function `add_subkey` tries to add `subkey` in the
    list of `children` of `key`. There are checks to do
    memory initialization as well. If a `subkey` is already
    there in `children`, then the `compatible_keys` function
    is used to check if the new key is "compatible" with
    the old "key". If yes, or if the `subkey` did not exist
    in the first place, a pointer to the existing or newly
    added subkey is returned respectively. Otherwise, it
    returns a NULL pointer on failure or buffer overflow.
*/
toml_key_t*
add_subkey(
    toml_key_t* key,
    toml_key_t* subkey
);

/*
    Function `compatible_keys` is used to decide if the
    re-definition of a key is acceptable by TOML specs.
    It uses the `toml_key_type` of the existing key and the
    key being re-defined and returns `true` if it is valid
    and `false` otherwise.
*/
bool
compatible_keys(
    toml_key_type_t existing,
    toml_key_type_t current
);

#endif

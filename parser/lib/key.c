#include "key.h"
#include "utils.h"
#include "value.h"

#include "khash.h"

#include <stdlib.h>
#include <string.h>

toml_key_t*
new_key( toml_key_type_t type ) {
    toml_key_t* k   = calloc( 1, sizeof( toml_key_t ) );
    k->type         = type;
    k->value        = NULL;
    k->idx          = -1;
    k->subkeys      = kh_init( str );
    memset( k->id, 0, TOML_MAX_ID_LENGTH );
    return k;
}

toml_key_t*
has_subkey(
    toml_key_t* key,
    toml_key_t* subkey
) {
    khiter_t k = kh_get( str, key->subkeys, subkey->id );
    if( k==kh_end( key->subkeys ) ) return NULL;
    return kh_value( key->subkeys, k );
}

toml_key_t*
add_subkey(
    toml_key_t* key,
    toml_key_t* subkey
) {
    toml_key_t* s = has_subkey( key, subkey );
    if( s ) {
        if( compatible_keys( s->type, subkey->type ) ) {
            // re-defining a TABLE as a TABLELEAF
            // is allowed only once
            if( subkey->type==TOML_TABLELEAF ) {
                s->type = TOML_TABLELEAF;
            }
            return s;
        }
        else {
            RETURN_IF_FAILED( 0,
                "failed to add subkey\n"
                "existing subkey - key: %s type: %d\n"
                "new subkey: key: %s type: %d\n", 
                s->id, ( int )( s->type ),
                subkey->id, ( int )( subkey->type )
            );
        }
    }
    if( kh_size( key->subkeys )<TOML_MAX_SUBKEYS ) {
        if( key->type==TOML_ARRAYTABLE ) {
            // since an ARRAYTABLE is a list of a map of key-value,
            // and re-defining an ARRAYTABLE means adding another map
            // of key-value to the list, we use the `value->arr`
            // attribute of the key to store each map of key-values
            toml_key_t* a = add_subkey( key->value->arr[ key->idx ]->data,
                                        subkey );
            return a;
        }
        else {
            int ret;
            khiter_t k = kh_put( str, key->subkeys, subkey->id, &ret );
            kh_value( key->subkeys, k ) = subkey;
            return subkey;
        }
    }
    else {
        LOG_ERR( "buffer overflow\n" );
    }
    return NULL;
}

bool
compatible_keys(
    toml_key_type_t existing,
    toml_key_type_t current
) {
    // re-definition rules
    // [existing]
    // [current]

    // `a = b`
    // `a = c`
    if( existing==TOML_KEYLEAF ) {
        return false;
    }
    // `[a.b]`
    // `[a.b]`
    if( existing==TOML_TABLELEAF && current==TOML_TABLELEAF ) {
        return false;
    }
    // `[a.b] or b.c = d`
    // `[a.b.e] or [b.e]`
    if( ( existing==TOML_TABLELEAF || existing==TOML_KEY ) &&
          current==TOML_TABLE ) {
        return true;
    }
    // `[a.b]`
    // `[a]`
    if( existing==TOML_TABLE && current==TOML_TABLELEAF ) {
        return true;
    }
    // `[[t]]`
    // `[t.s]`
    if( existing==TOML_ARRAYTABLE && current==TOML_TABLE ) {
        return true;
    }
    if( current==existing ) {
        return true;
    }
    return false;
}

void
delete_key( toml_key_t* key ) {
    if( !key ) return;
    kh_destroy( str, key->subkeys );
    if( key->value ) {
        delete_value( key->value );
    }
    free( key );
}

#include "key.h"
#include "utils.h"
#include "value.h"

#include <stdlib.h>
#include <string.h>

toml_key_t* new_key( toml_key_type_t type )
{
    toml_key_t* k   = calloc( 1, sizeof( toml_key_t ) );
    k->type         = type;
    k->last         = NULL;
    k->value        = NULL;
    k->idx          = -1;
    memset( k->id, 0, MAX_ID_LENGTH );
    return k;
}

toml_key_t* has_subkey(
    toml_key_t* key,
    toml_key_t* subkey
)
{
    for( toml_key_t** iter=key->subkeys; iter<key->last; iter++ )
    {
        if ( strcmp( ( *iter )->id, subkey->id )==0 )
        {
            return *iter;
        }
    }
    return NULL;
}

toml_key_t* add_subkey(
    toml_key_t* key,
    toml_key_t* subkey
)
{
    if( key->last==NULL )
    {
        // first time adding a subkey
        memset( key->subkeys, 0, MAX_NUM_SUBKEYS );
        key->last = key->subkeys;
    }
    toml_key_t* s = has_subkey( key, subkey );
    if( s )
    {
        if( compatible_keys( s->type, subkey->type ) )
        {
            // re-defining a TABLE as a TABLELEAF
            // is allowed only once
            if( subkey->type==TABLELEAF )
            {
                s->type = TABLELEAF;
            }
            return s;
        }
        else
        {
            RETURN_IF_FAILED( 0,
                "failed to add subkey\n"
                "existing subkey - key: %s type: %d\n"
                "new subkey: key: %s type: %d\n", 
                s->id, ( int )( s->type ),
                subkey->id, ( int )( subkey->type )
            );
        }
    }
    if( key->last-key->subkeys<MAX_NUM_SUBKEYS )
    {
        if( key->type==ARRAYTABLE )
        {
            // since an ARRAYTABLE is a list of a map of key-value,
            // and re-defining an ARRAYTABLE means adding another map
            // of key-value to the list, we use the `value->arr`
            // attribute of the key to store each map of key-values
            toml_key_t* a = add_subkey( key->value->arr[ key->idx ]->data, subkey );
            return a;
        }
        else
        {
            *( key->last ) = subkey;
            key->last++;
            return subkey;
        }
    }
    else
    {
        LOG_ERR( "buffer overflow\n" );
    }
    return NULL;
}

bool compatible_keys(
    toml_key_type_t existing,
    toml_key_type_t current
)
{
    // re-definition rules
    // [existing]
    // [current]

    // `a = b`
    // `a = c`
    if( existing==KEYLEAF )
    {
        return false;
    }
    // `[a.b]`
    // `[a.b]`
    if( existing==TABLELEAF && current==TABLELEAF )
    {
        return false;
    }
    // `[a.b] or b.c = d`
    // `[a.b.e] or [b.e]`
    if( ( existing==TABLELEAF || existing==KEY ) && current==TABLE )
    {
        return true;
    }
    // `[a.b]`
    // `[a]`
    if( existing==TABLE && current==TABLELEAF )
    {
        return true;
    }
    // `[[t]]`
    // `[t.s]`
    if( existing==ARRAYTABLE && current==TABLE )
    {
        return true;
    }
    if( current==existing )
    {
        return true;
    }
    return false;
}

void delete_key( toml_key_t* key )
{
    if( !key ) return;
    if( key->last )
    {
        for( toml_key_t** iter=key->subkeys; iter<key->last; iter++ )
        {
            delete_key( *iter );
        }
    }
    if( key->value )
    {
        delete_value( key->value );
    }
    free( key );
}

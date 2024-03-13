#include "parse_keys.h"
#include "parse_values.h"
#include "parse_utils.h"

#include "lib/key.h"
#include "lib/value.h"
#include "lib/utils.h"

#include <string.h>

toml_key_t* parse_barekey(
    tokenizer_t*    tok,
    char            end,
    toml_key_type_t branch,
    toml_key_type_t leaf
)
{
    char    id[ TOML_MAX_ID_LENGTH ] = { 0 };
    int     idx                 = 0;
    bool    done                = false;

    while( has_token( tok ) )
    {
        RETURN_IF_FAILED( idx<TOML_MAX_ID_LENGTH, "buffer overflow\n" );
        if( is_dot( get_token( tok ) ) )
        {
            RETURN_IF_FAILED( idx!=0, "key cannot be empty\n" );
            toml_key_t* subkey = new_key( branch );
            memcpy( subkey->id, id, strlen( id ) );
            return subkey;
        }
        else if( get_token( tok )==end )
        {
            RETURN_IF_FAILED( idx!=0, "key cannot be empty\n" );
            toml_key_t* subkey = new_key( leaf );
            memcpy( subkey->id, id, strlen( id ) );
            return subkey;
        }
        else if( is_whitespace( get_token( tok ) ) )
        {
            // bare keys cannot contain whitespace inside each
            // key, so we use the `done` variable to track it
            done = true;
            parse_whitespace( tok );
        }
        else if( is_bare_ascii( get_token( tok ) ) && !done )
        {
            id[ idx++ ] = get_token( tok );
            next_token( tok );
        }
        else
        {
            LOG_ERR( "unknown character %c\n", get_token( tok ) );
            break;
        }
    }
    return NULL;
}

toml_key_t* parse_basicquotedkey(
    tokenizer_t*    tok,
    char            end,
    toml_key_type_t branch,
    toml_key_type_t leaf
)
{
    char    id[ TOML_MAX_ID_LENGTH ] = { 0 };
    int     idx                 = 0;

    while( has_token( tok ) )
    {
        RETURN_IF_FAILED( idx<TOML_MAX_ID_LENGTH, "buffer overflow\n" );
        if( is_basicstringstart( get_token( tok ) ) )
        {
            next_token( tok );
            if( is_whitespace( get_token( tok ) ) )
            {
                parse_whitespace( tok );
            }
            if( is_dot( get_token( tok ) ) )
            {
                toml_key_t* subkey = new_key( branch );
                memcpy( subkey->id, id, strlen( id ) );
                return subkey;
            }
            else if( get_token( tok )==end )
            {
                toml_key_t* subkey = new_key( leaf );
                memcpy( subkey->id, id, strlen( id ) );
                return subkey;
            }
            LOG_ERR( "unknown character %c after end of key\n", get_token( tok ) );
            break;
        }
        else if( is_newline( get_token( tok ) ) )
        {
            LOG_ERR( "unescaped newline while parsing key\n" );
            break;
        }
        else if( is_escape( get_token( tok ) ) )
        {
            next_token( tok );
            char escaped[ 5 ] = { 0 };
            int c             = parse_escape( tok, escaped, 5 );
            RETURN_IF_FAILED( c!=0, "unknown escape sequence \\%c\n", get_token( tok ) );
            RETURN_IF_FAILED( c<5,  "parsed escape sequence is too long\n" );
            for( int i=0; i<c; i++ )
            {
                id[ idx++ ] = escaped[ i ];
                RETURN_IF_FAILED( idx<TOML_MAX_ID_LENGTH, "buffer overflow\n" );
            }
            // parse_escape will parse everything and move on to the next token
            // so we call backtrack here to offset the next_token call outside
            backtrack( tok, 1 );
        }
        else if( is_control( get_token( tok ) ) )
        {
            LOG_ERR( "control characters need to be escaped\n" );
            break;
        }
        else
        {
            id[ idx++ ] = get_token( tok );
        }
        next_token( tok );
    }
    return NULL;
}

toml_key_t* parse_literalquotedkey(
    tokenizer_t*    tok,
    char            end,
    toml_key_type_t branch,
    toml_key_type_t leaf
)
{
    char    id[ TOML_MAX_ID_LENGTH ] = { 0 };
    int     idx                 = 0;

    while( has_token( tok ) )
    {
        RETURN_IF_FAILED( idx<TOML_MAX_ID_LENGTH, "buffer overflow\n" );
        if( is_literalstringstart( get_token( tok ) ) )
        {
            next_token( tok );
            if( is_whitespace( get_token( tok ) ) )
            {
                parse_whitespace( tok );
            }
            if( is_dot( get_token( tok ) ) )
            {
                toml_key_t* subkey = new_key( branch );
                memcpy( subkey->id, id, strlen( id ) );
                return subkey;
            }
            else if( get_token( tok )==end )
            {
                toml_key_t* subkey = new_key( leaf );
                memcpy( subkey->id, id, strlen( id ) );
                return subkey;
            }
            LOG_ERR( "unknown character %c after end of key\n", get_token( tok ) );
            break;
        }
        else if( is_newline( get_token( tok ) ) )
        {
            LOG_ERR( "unescaped newline while parsing key\n" );
            break;
        }
        else if( is_control_literal( get_token( tok ) ) )
        {
            LOG_ERR( "control characters need to be escaped\n" );
            break;
        }
        else
        {
            id[ idx++ ] = get_token( tok );
        }
        next_token( tok );
    }
    return NULL;
}

toml_key_t* parse_key(
    tokenizer_t*    tok,
    toml_key_t*     key,
    bool            expecting
)
{
    while( has_token( tok ) )
    {
        if( is_equal( get_token( tok ) ) )
        {
            RETURN_IF_FAILED( !expecting, "found = while expecting a key\n" );
            next_token( tok );
            return key;
        }
        else if( is_dot( get_token( tok ) ) )
        {
            RETURN_IF_FAILED( !expecting, "found . while expecting a key\n" );
            next_token( tok );
            return parse_key( tok, key, true );
        }
        else if( is_whitespace( get_token( tok ) ) )
        {
            parse_whitespace( tok );
        }
        else if( is_basicstringstart( get_token( tok ) ) )
        {
            next_token( tok );
            toml_key_t* subkey = parse_basicquotedkey( tok, '=', TOML_KEY, TOML_KEYLEAF );
            RETURN_IF_FAILED( subkey, "failed to parse basic quoted key\n" );
            subkey = add_subkey( key, subkey );
            RETURN_IF_FAILED( subkey, "failed to add subkey to key %s\n", key->id );
            return parse_key( tok, subkey, false );
        }
        else if( is_literalstringstart( get_token( tok ) ) )
        {
            next_token( tok );
            toml_key_t* subkey = parse_literalquotedkey( tok, '=', TOML_KEY, TOML_KEYLEAF );
            RETURN_IF_FAILED( subkey, "failed to parse literal quoted key\n" );
            subkey = add_subkey( key, subkey );
            RETURN_IF_FAILED( subkey, "failed to add subkey to key %s\n", key->id );
            return parse_key( tok, subkey, false );
        }
        else
        {
            toml_key_t* subkey = parse_barekey( tok, '=', TOML_KEY, TOML_KEYLEAF );
            RETURN_IF_FAILED( subkey, "failed to parse bare key\n" );
            subkey = add_subkey( key, subkey );
            RETURN_IF_FAILED( subkey, "failed to add subkey to key %s\n", key->id );
            return parse_key( tok, subkey, false );
        }
    }
    return NULL;
}

toml_key_t* parse_table(
    tokenizer_t*    tok,
    toml_key_t*     key,
    bool            expecting
)
{
    while( has_token( tok ) )
    {
        if( is_tableend( get_token( tok ) ) )
        {
            RETURN_IF_FAILED( !expecting, "found ] while expecting a key\n" );
            next_token( tok );
            return key;
        }
        else if( is_dot( get_token( tok ) ) )
        {
            RETURN_IF_FAILED( !expecting, "found . while expecting a key\n" );
            next_token( tok );
            return parse_table( tok, key, true );
        }
        else if( is_whitespace( get_token( tok ) ) )
        {
            parse_whitespace( tok );
        }
        else if( is_basicstringstart( get_token( tok ) ) )
        {
            next_token( tok );
            toml_key_t* subkey = parse_basicquotedkey( tok, ']', TOML_TABLE, TOML_TABLELEAF );
            RETURN_IF_FAILED( subkey, "failed to parse basic quoted key\n" );
            subkey = add_subkey( key, subkey );
            RETURN_IF_FAILED( subkey, "failed to add key to subkey %s\n", key->id );
            return parse_table( tok, subkey, false );
        }
        else if( is_literalstringstart( get_token( tok ) ) )
        {
            next_token( tok );
            toml_key_t* subkey = parse_literalquotedkey( tok, ']', TOML_TABLE, TOML_TABLELEAF );
            RETURN_IF_FAILED( subkey, "failed to parse literal quoted key\n" );
            subkey = add_subkey( key, subkey );
            RETURN_IF_FAILED( subkey, "failed to add key to subkey %s\n", key->id );
            return parse_table( tok, subkey, false );
        }
        else
        {
            toml_key_t* subkey = parse_barekey( tok, ']', TOML_TABLE, TOML_TABLELEAF );
            RETURN_IF_FAILED( subkey, "failed to parse bare key\n" );
            subkey = add_subkey( key, subkey );
            RETURN_IF_FAILED( subkey, "failed to add key to subkey %s\n", key->id );
            return parse_table( tok, subkey, false );
        }
    }
    return NULL;
}

toml_key_t* parse_arraytable(
    tokenizer_t*    tok,
    toml_key_t*     key,
    bool            expecting
)
{
    while( has_token( tok ) )
    {
        if( is_tableend( get_token( tok ) ) )
        {
            RETURN_IF_FAILED( !expecting, "found ] while expecting a key\n" );
            next_token( tok );
            RETURN_IF_FAILED( is_tableend( get_token( tok ) ), "found ] while expecting a key\n" );
            next_token( tok );
            return key;
        }
        else if( is_dot( get_token( tok ) ) )
        {
            RETURN_IF_FAILED( !expecting, "found . while expecting a key\n" );
            next_token( tok );
            return parse_arraytable( tok, key, true );
        }
        else if( is_whitespace( get_token( tok ) ) )
        {
            parse_whitespace( tok );
        }
        else if( is_basicstringstart( get_token( tok ) ) )
        {
            next_token( tok );
            toml_key_t* subkey = parse_basicquotedkey( tok, ']', TOML_TABLE, TOML_ARRAYTABLE );
            RETURN_IF_FAILED( subkey, "failed to parse basic quoted key\n" );
            subkey = add_subkey( key, subkey );
            RETURN_IF_FAILED( subkey, "failed to add key to subkey %s\n", key->id );
            return parse_arraytable( tok, subkey, false );
        }
        else if( is_literalstringstart( get_token( tok ) ) )
        {
            next_token( tok );
            toml_key_t* subkey = parse_literalquotedkey( tok, ']', TOML_TABLE, TOML_ARRAYTABLE );
            RETURN_IF_FAILED( subkey, "failed to parse literal quoted key\n" );
            subkey = add_subkey( key, subkey );
            RETURN_IF_FAILED( subkey, "failed to add key to subkey %s\n", key->id );
            return parse_arraytable( tok, subkey, false );
        }
        else
        {
            toml_key_t* subkey = parse_barekey( tok, ']', TOML_TABLE, TOML_ARRAYTABLE );
            RETURN_IF_FAILED( subkey, "failed to parse bare key\n" );
            subkey = add_subkey( key, subkey );
            RETURN_IF_FAILED( subkey, "failed to add key to subkey %s\n", key->id );
            return parse_arraytable( tok, subkey, false );
        }
    }
    return NULL;
}

toml_key_t* parse_keyval(
    tokenizer_t*    tok,
    toml_key_t*     key,
    toml_key_t*     root
)
{
    if( is_commentstart( get_token( tok ) ) )
    {
        bool ok = parse_comment( tok );
        RETURN_IF_FAILED( ok, "invalid comment\n" );
        return key;
    }
    else if( is_whitespace( get_token( tok ) ) )
    {
        parse_whitespace( tok );
        return key;
    }
    else if( parse_newline( tok ) )
    {
        next_token( tok );
        return key;
    }
    else if( is_tablestart( get_token( tok ) ) )
    {
        next_token( tok );
        toml_key_t* table;
        // [[ means we are parsing an arraytable
        if( is_tablestart( get_token( tok ) ) )
        {
            next_token( tok );
            table = parse_arraytable( tok, root, true );
            RETURN_IF_FAILED( table, "failed to parse array of tables\n" );
            // Since an arraytable is a map of key-value pairs, we
            // store it in the `value->arr` attribute of the `key`.
            // Each redefinition marks an new element in that array.
            // The key-value pairs are added to the `subkeys` of a
            // "pseudo" key that lives at `table->value->arr[ table->idx ].
            if( table->value==NULL )
            {
                table->value = new_array();
            }
            RETURN_IF_FAILED( table->idx<TOML_MAX_ARRAY_LENGTH-1, "buffer overflow\n" );
            table->value->arr[ ++( table->idx ) ] = new_inline_table( new_key( TOML_TABLE ) );
        }
        else
        {
            table = parse_table( tok, root, true );
            RETURN_IF_FAILED( table, "failed to parse table\n" );
        }
        return table;
    }
    else if(
        get_prev( tok )=='\0' ||
        is_newline( get_prev( tok ) ) ||
        // ignore white space found at the beginning of
        // a line while parsing a key
        ( is_whitespace( get_prev( tok ) ) &&
        tok->newline )
    )
    {
        toml_key_t* subkey  = parse_key( tok, key, true );
        RETURN_IF_FAILED( subkey, "failed to parse key\n" );
        toml_value_t* v     = parse_value( tok, "# \n" );
        RETURN_IF_FAILED( v, "failed to parse value\n" );
        // If we parsed an inlinetable, to keep it in sync
        // with our datastructure, we add the keys from the
        // parsed key-value pairs as `subkeys` of the "active"
        // `key`. Since the inline table is defined as
        // `a = b`, the type would be a KEYLEAF. Since KEYLEAF
        // re-definitions are not allowed, we "unlock" it as a
        // KEY, add the `subkeys` and "lock" it again as a
        // `KEYLEAF` to prevent re-definition.
        if( v->type==TOML_INLINETABLE )
        {
            toml_key_t* h   = ( toml_key_t * )( v->data );
            subkey->type    = TOML_KEY;
            for( khiter_t ki=kh_begin( h->subkeys ); ki!=kh_end( h->subkeys ); ++ki )
            {
                if( kh_exist( h->subkeys, ki ) )
                {
                    toml_key_t* e = add_subkey( subkey, kh_value( h->subkeys, ki ) );
                    RETURN_IF_FAILED( e, "could not add inline table key %s\n", kh_value( h->subkeys, ki )->id );
                }
            }
            subkey->type    = TOML_KEYLEAF;
        }
        else
        {
            subkey->value   = v;
        }
        parse_whitespace( tok );
        return key; 
    }
    else
        LOG_ERR( "unhandled character %c\n", get_token( tok ) );
    return NULL;
}

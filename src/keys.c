#include "keys.h"
#include "values.h"
#include "utils.h"

#include "../lib/key.h"
#include "../lib/value.h"
#include "../lib/utils.h"

#include <string.h>

#define KEY_INIT()                              \
    char id[ MAX_ID_LENGTH ];                   \
    memset( id, 0, MAX_ID_LENGTH );             \
    size_t idx = 0;

#define RETURN_SUBKEY( TYPE, CSTR )             \
    key_t* subkey = new_key( TYPE );            \
    // NOTE(AJB) This looks like a buffer-overflow ... why is the string length always less than sizeof(subkey->id) ??
    memcpy( subkey->id, CSTR, strlen( CSTR ) );     \
    return subkey;

// NOTE(AJB): I really think this would be cleaner if you did a proper tokenizer. See comments in tokenizer.c.
// At this stage, you really don't want to be thinking about whitespace and comments. A barekey is just
// "identifier".. trivial.
key_t* parse_barekey(
    tokenizer_t*    tok,
    char            end,
    key_type_t      branch,
    key_type_t      leaf
)
{
    KEY_INIT()
    bool done = false;
    while( has_token( tok ) )
    {
        FAIL_BREAK( idx<MAX_ID_LENGTH, "buffer overflow\n" )
        if( is_dot( get_token( tok ) ) )
        {
            FAIL_BREAK( idx!=0, "key cannot be empty\n" )
            RETURN_SUBKEY( branch, id )
        }
        else if( get_token( tok )==end )
        {
            FAIL_BREAK( idx!=0, "key cannot be empty\n" )
            RETURN_SUBKEY( leaf, id )
        }
        else if( is_whitespace( get_token( tok ) ) )
        {
            done = true;
            parse_whitespace( tok );
        }
        else if( is_ascii( get_token( tok ) ) && !done )
        {
            id[ idx++ ] = get_token( tok );
            next_token( tok );
        }
        else
            LOG_ERR_BREAK( "unknown character %c\n", get_token( tok ) )
    }
    return NULL;
}

key_t* parse_basicquotedkey(
    tokenizer_t*    tok,
    char            end,
    key_type_t      branch,
    key_type_t      leaf
)
{
    KEY_INIT()
    while( has_token( tok ) )
    {
        FAIL_BREAK( idx<MAX_ID_LENGTH, "buffer overflow\n" )
        if( idx==0 && is_basicstringstart( get_token( tok ) ) )
            ;
        else if( idx>0 && is_basicstringstart( get_token( tok ) ) )
        {
            next_token( tok );
            if( is_whitespace( get_token( tok ) ) )
                parse_whitespace( tok );
            if( is_dot( get_token( tok ) ) )
            {
                FAIL_BREAK( idx!=0, "key cannot be empty\n" )
                RETURN_SUBKEY( branch, id )
            }
            else if( get_token( tok )==end )
            {
                FAIL_BREAK( idx!=0, "key cannot be empty\n" )
                RETURN_SUBKEY( leaf, id )
            }
            LOG_ERR_BREAK( "unknown character %c after end of key\n", get_token( tok ) )
        }
        else if( is_escape( get_token( tok ) ) )
        {
            next_token( tok );
            char c = escape( get_token( tok ) );
            FAIL_BREAK( c!=0, "unknown escape sequence \\%c\n", get_token( tok ) )
            id[ idx++ ] = c;
        }
        else
            id[ idx++ ] = get_token( tok );
        next_token( tok );
    }
    return NULL;
}

key_t* parse_literalquotedkey(
    tokenizer_t*    tok,
    char            end,
    key_type_t      branch,
    key_type_t      leaf
)
{
    KEY_INIT()
    while( has_token( tok ) )
    {
        FAIL_BREAK( idx<MAX_ID_LENGTH, "buffer overflow\n" )
        if( idx==0 && is_literalstringstart( get_token( tok ) ) )
            ;
        else if( idx>0 && is_literalstringstart( get_token( tok ) ) )
        {
            next_token( tok );
            if( is_whitespace( get_token( tok ) ) )
                parse_whitespace( tok );
            if( is_dot( get_token( tok ) ) )
            {
                FAIL_BREAK( idx!=0, "key cannot be empty\n" )
                RETURN_SUBKEY( branch, id )
            }
            else if( get_token( tok )==end )
            {
                FAIL_BREAK( idx!=0, "key cannot be empty\n" )
                RETURN_SUBKEY( leaf, id )
            }
            LOG_ERR_BREAK( "unknown character %c after end of key\n", get_token( tok ) )
        }
        else
            id[ idx++ ] = get_token( tok );
        next_token( tok );
    }
    return NULL;
}

key_t* parse_key(
    tokenizer_t*    tok,
    key_t*          key,
    bool            expecting
)
{
    while( has_token( tok ) )
    {
        if( is_equal( get_token( tok ) ) )
        {
            FAIL_BREAK( !expecting, "found = while expecting a key\n" )
            next_token( tok );
            return key;
        }
        else if( is_dot( get_token( tok ) ) )
        {
            FAIL_BREAK( !expecting, "found . while expecting a key\n" )
            next_token( tok );
            return parse_key( tok, key, true );
        }
        else if( is_whitespace( get_token( tok ) ) )
            parse_whitespace( tok );
        else if( is_basicstringstart( get_token( tok ) ) )
        {
            key_t* subkey = parse_basicquotedkey( tok, '=', KEY, KEYLEAF );
            FAIL_BREAK( subkey, "failed to parse basic quoted key\n" )
            subkey = add_subkey( key, subkey );
            FAIL_BREAK( subkey, "failed to add subkey to key %s\n", key->id )
            return parse_key( tok, subkey, false );
        }
        else if( is_literalstringstart( get_token( tok ) ) )
        {
            key_t* subkey = parse_literalquotedkey( tok, '=', KEY, KEYLEAF );
            FAIL_BREAK( subkey, "failed to parse literal quoted key\n" )
            subkey = add_subkey( key, subkey );
            FAIL_BREAK( subkey, "failed to add subkey to key %s\n", key->id )
            return parse_key( tok, subkey, false );
        }
        else
        {
            key_t* subkey = parse_barekey( tok, '=', KEY, KEYLEAF );
            FAIL_BREAK( subkey, "failed to parse bare key\n" )
            subkey = add_subkey( key, subkey );
            FAIL_BREAK( subkey, "failed to add subkey to key %s\n", key->id )
            return parse_key( tok, subkey, false );
        }
    }
    return NULL;
}

key_t* parse_table(
    tokenizer_t*    tok,
    key_t*          key,
    bool            expecting
)
{
    while( has_token( tok ) )
    {
        if( is_tableend( get_token( tok ) ) )
        {
            FAIL_BREAK( !expecting, "found ] while expecting a key\n" )
            next_token( tok );
            return key;
        }
        else if( is_dot( get_token( tok ) ) )
        {
            FAIL_BREAK( !expecting, "found . while expecting a key\n" )
            next_token( tok );
            return parse_table( tok, key, true );
        }
        else if( is_whitespace( get_token( tok ) ) )
            parse_whitespace( tok );
        else if( is_basicstringstart( get_token( tok ) ) )
        {
            key_t* subkey = parse_basicquotedkey( tok, ']', TABLE, TABLELEAF );
            FAIL_BREAK( subkey, "failed to parse basic quoted key\n" )
            subkey = add_subkey( key, subkey );
            FAIL_BREAK( subkey, "failed to add key to subkey %s\n", key->id )
            return parse_table( tok, subkey, false );
        }
        else if( is_literalstringstart( get_token( tok ) ) )
        {
            key_t* subkey = parse_literalquotedkey( tok, ']', TABLE, TABLELEAF );
            FAIL_BREAK( subkey, "failed to parse literal quoted key\n" )
            subkey = add_subkey( key, subkey );
            FAIL_BREAK( subkey, "failed to add key to subkey %s\n", key->id )
            return parse_table( tok, subkey, false );
        }
        else
        {
            key_t* subkey = parse_barekey( tok, ']', TABLE, TABLELEAF );
            FAIL_BREAK( subkey, "failed to parse bare key\n" )
            subkey = add_subkey( key, subkey );
            FAIL_BREAK( subkey, "failed to add key to subkey %s\n", key->id )
            return parse_table( tok, subkey, false );
        }
    }
    return NULL;
}

key_t* parse_arraytable(
    tokenizer_t*    tok,
    key_t*          key,
    bool            expecting
)
{
    while( has_token( tok ) )
    {
        if( is_tableend( get_token( tok ) ) )
        {
            FAIL_BREAK( !expecting, "found ] while expecting a key\n" )
            next_token( tok );
            FAIL_BREAK( is_tableend( get_token( tok ) ), "found ] while expecting a key\n" )
            next_token( tok );
            return key;
        }
        else if( is_dot( get_token( tok ) ) )
        {
            FAIL_BREAK( !expecting, "found . while expecting a key\n" )
            next_token( tok );
            return parse_arraytable( tok, key, true );
        }
        else if( is_whitespace( get_token( tok ) ) )
            parse_whitespace( tok );
        else if( is_basicstringstart( get_token( tok ) ) )
        {
            key_t* subkey = parse_basicquotedkey( tok, ']', TABLE, ARRAYTABLE );
            FAIL_BREAK( subkey, "failed to parse basic quoted key\n" )
            subkey = add_subkey( key, subkey );
            FAIL_BREAK( subkey, "failed to add key to subkey %s\n", key->id )
            return parse_arraytable( tok, subkey, false );
        }
        else if( is_literalstringstart( get_token( tok ) ) )
        {
            key_t* subkey = parse_literalquotedkey( tok, ']', TABLE, ARRAYTABLE );
            FAIL_BREAK( subkey, "failed to parse literal quoted key\n" )
            subkey = add_subkey( key, subkey );
            FAIL_BREAK( subkey, "failed to add key to subkey %s\n", key->id )
            return parse_arraytable( tok, subkey, false );
        }
        else
        {
            key_t* subkey = parse_barekey( tok, ']', TABLE, ARRAYTABLE );
            FAIL_BREAK( subkey, "failed to parse bare key\n" )
            subkey = add_subkey( key, subkey );
            FAIL_BREAK( subkey, "failed to add key to subkey %s\n", key->id )
            return parse_arraytable( tok, subkey, false );
        }
    }
    return NULL;
}

key_t* parse_keyval(
    tokenizer_t*    tok,
    key_t*          key,
    key_t*          root
)
{
  // NOTE(AJB) This general flow makes sense. Very LL(1) recersive-descent predictive parser.
  // Step (1) check some prefix, Step (2) recursing into it.
  // But again, you should be doing it over tokens
  if( is_commentstart( get_token( tok ) ) )
    {
        parse_comment( tok );
        return key;
    }
    else if( is_whitespace( get_token( tok ) ) )
    {
        parse_whitespace( tok );
        return key;
    }
    else if( is_newline( get_token( tok ) ) )
    {
        next_token( tok );
        return key;
    }
    else if( is_tablestart( get_token( tok ) ) )
    {
        next_token( tok );
        key_t* table;
        if( is_tablestart( get_token( tok ) ) )
        {
            next_token( tok );
            table = parse_arraytable( tok, root, true );
            FAIL_RETURN( table, "failed to parse array of tables\n" )
            if( table->value==NULL )
                table->value = new_array();
            table->value->arr[ ++( table->idx ) ] = new_inline_table( new_key( TABLE ) );
        }
        else
        {
            table = parse_table( tok, root, true );
            FAIL_RETURN( table, "failed to parse table\n" )
        }
        return table;
    }
    else if(
        get_prev( tok )=='\0' ||
        is_newline( get_prev( tok ) ) ||
        ( is_whitespace( get_prev( tok ) ) &&
        tok->newline )
    )
    {
        key_t* subkey = parse_key( tok, key, true );
        FAIL_RETURN( subkey, "failed to parse key\n" )
        value_t* v = parse_value( tok, "# \n" );
        FAIL_RETURN( v, "failed to parse value\n" )
        if( v->type==INLINETABLE )
        {
            key_t* h = ( key_t * )( v->data );
            subkey->type = KEY;
            for( key_t** iter=h->subkeys; iter<h->last; iter++ )
            {
                key_t* e = add_subkey( subkey, *iter );
                FAIL_RETURN( e, "could not add inline table key %s\n", ( *iter )->id )
            }
            subkey->type = KEYLEAF;
        }
        else
            subkey->value = v;
        parse_whitespace( tok );
        return key;
    }
    else
        LOG_ERR_RETURN( "unhandled character %c\n", get_token( tok ) )
}

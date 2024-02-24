#include "tokenizer.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>

tokenizer_t* new_tokenizer( FILE* input )
{
    tokenizer_t* tok = calloc( 1, sizeof( tokenizer_t ) );
    tok->stream = input;
    tok->cursor = 0;
    tok->token = '\0';
    tok->prev = '\0';
    tok->prev_prev = '\0';
    tok->line = 1;
    tok->col = 0;
    tok->has_token = true;
    return tok;
}

size_t next_token( tokenizer_t* tok )
{
    fseek( tok->stream, tok->cursor, SEEK_SET );
    if( tok->prev!='\0' )
        tok->prev_prev = tok->prev;
    if( tok->token!='\0' )
        tok->prev = tok->token;
    if( tok->has_token || tok->cursor==0 )
    {
        char c = fgetc( tok->stream );
        // if we parsed some non-whitespace character since we saw
        // the newline, then we aren't on a newline anymore
        if( tok->prev && tok->prev!=' ' && tok->prev!='\t' && tok->prev!='\n' )
            tok->newline = false;
        if( c=='\n' )
            tok->newline = true;
        if( tok->prev=='\n' )
        {
            tok->line++;
            tok->col = 0;
        }
        else
            tok->col++;
        if( c==EOF )
        {
            tok->token = '\0';
            tok->has_token = false;
        }
        else
        {
            tok->token = c;
            tok->has_token = true;
        }
        tok->cursor++;
        return 1;
    }
    return 0;
}

void backtrack(
    tokenizer_t*    tok,
    int             count
)
{
    if( count>0 && tok->cursor>count+2 )
    {
        tok->cursor -= (count+2);
        tok->has_token = true;
        next_token( tok );
        next_token( tok );
        // incorrect
        tok->col -= count;
    }
    else
        LOG_ERR( "not enough characters to backtrack %d\n", count );
}

bool has_token( tokenizer_t* tok )
{
    return tok->has_token;
}

char get_token( tokenizer_t* tok )
{
    return tok->token;
}

char get_prev( tokenizer_t* tok )
{
    return tok->prev;
}

char get_prev_prev( tokenizer_t* tok )
{
    return tok->prev_prev;
}

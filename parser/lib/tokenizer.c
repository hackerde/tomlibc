#include "tokenizer.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>

tokenizer_t* new_tokenizer( FILE* input )
{
    tokenizer_t* tok    = calloc( 1, sizeof( tokenizer_t ) );
    tok->stream         = input;
    tok->cursor         = 0;
    tok->token          = '\0';
    tok->prev           = '\0';
    tok->prev_prev      = '\0';
    tok->line           = 0;
    tok->col            = 0;
    tok->has_token      = true;
    memset( tok->lines, 0, MAX_NUM_LINES );
    return tok;
}

int next_token( tokenizer_t* tok )
{
    fseek( tok->stream, tok->cursor, SEEK_SET );
    if( tok->prev!='\0' )
    {
        tok->prev_prev  = tok->prev;
    }
    if( tok->token!='\0' )
    {
        tok->prev       = tok->token;
    }
    if( tok->has_token || tok->cursor==0 )
    {
        char c          = fgetc( tok->stream );
        // if we parsed some non-whitespace character since we saw
        // the newline, then we aren't on a newline anymore
        if( tok->prev && tok->prev!=' ' &&
            tok->prev!='\t' && tok->prev!='\n'
        )
        {
            tok->newline = false;
        }
        if( c=='\n' )
        {
            tok->newline = true;
        }
        if( tok->prev=='\n' )
        {
            if( tok->line<MAX_NUM_LINES )
            {
                tok->lines[ tok->line ] = tok->col;
            }
            tok->line++;
            tok->col        = 0;
        }
        else
        {
            tok->col++;
        }
        if( c==EOF )
        {
            tok->token      = '\0';
            tok->has_token  = false;
        }
        else
        {
            tok->token      = c;
            tok->has_token  = true;
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
    int pre_count           = count+2;
    if( count>0 && tok->cursor>pre_count )
    {
        tok->cursor         -= pre_count;
        tok->has_token      = true;
        int col             = tok->col;
        while( tok->line>=0 && pre_count>col )
        {
            pre_count       -= col;
            col             = tok->lines[ --tok->line ];
        }
        tok->col            = col-pre_count;
        if( tok->line<0 )   tok->line = 0;
        if( tok->col<0 )    tok->col  = 0;
        next_token( tok );
        next_token( tok );
    }
    else
    {
        LOG_ERR( "not enough characters to backtrack %d\n", count );
    }
}

long get_input_size( tokenizer_t* tok )
{
    fseek( tok->stream, 0L, SEEK_END );
    long sz = ftell( tok->stream );
    fseek( tok->stream, tok->cursor, SEEK_SET );
    return sz;
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

void delete_tokenizer( tokenizer_t* tok )
{
    free( tok );
}

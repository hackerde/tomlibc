#include "tokenizer.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>

tokenizer_t*
new_tokenizer( char* input ) {
    tokenizer_t* tok    = calloc( 1, sizeof( tokenizer_t ) );
    tok->input          = input;
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

int
next_token( tokenizer_t* tok ) {
    tok->prev_prev  = tok->prev;
    tok->prev       = tok->token;
    if( tok->has_token || tok->cursor==0 ) {
        tok->token      = tok->stream[ tok->cursor++ ];
        // if we parsed some non-whitespace character since we saw
        // the newline, then we aren't on a newline anymore
        if( tok->newline && tok->prev && tok->prev!=' ' &&
            tok->prev!='\t' && tok->prev!='\n'
        ) {
            tok->newline = false;
        }
        if( tok->token=='\n' ) {
            tok->newline = true;
        }
        if( tok->prev=='\n' ) {
            if( tok->line<MAX_NUM_LINES ) {
                tok->lines[ tok->line ] = tok->col;
            }
            tok->line++;
            tok->col        = 0;
        }
        else {
            tok->col++;
        }
        if( tok->token==EOF ) {
            tok->token      = '\0';
            tok->has_token  = false;
        }
        return 1;
    }
    return 0;
}

void
backtrack(
    tokenizer_t*    tok,
    int             count
) {
    int pre_count           = count+2;
    if( count>0 && tok->cursor>pre_count ) {
        tok->cursor         -= pre_count;
        tok->has_token      = true;
        int col             = tok->col;
        while( tok->line>=0 && pre_count>col ) {
            pre_count       -= col;
            col             = tok->lines[ --tok->line ];
        }
        tok->col            = col-pre_count;
        if( tok->line<0 )   tok->line = 0;
        if( tok->col<0  )   tok->col  = 0;
        next_token( tok );
        next_token( tok );
    }
    else {
        LOG_ERR( "not enough characters to backtrack %d\n", count );
    }
}

bool
load_input( tokenizer_t* tok ) {
    #define MAX_FILE_SIZE 1073741824
    FILE* stream;
    if( tok->input ) {
        stream = fopen( tok->input, "r" );
    }
    else {
        stream = stdin;
    }
    if( !stream ) {
        LOG_ERR( "could not open input stream\n" );
        return false;
    }
    fseek( stream, 0L, SEEK_END );
    long   size = ftell( stream );
    fseek( stream, 0L, SEEK_SET );
    if( size>=MAX_FILE_SIZE ) {
        LOG_ERR( "input size is too big\n" );
        return false;
    }
    char* buffer    = calloc( 1, size+1 );
    if( size>0 && 1!=fread( buffer, size, 1, stream ) ) {
        LOG_ERR( "could not read input\n" );
        return false;
    }
    buffer[ size ]  = EOF;
    tok->stream     = buffer;
    #undef MAX_FILE_SIZE
    return true;
}

bool
has_token( tokenizer_t* tok ) {
    return tok->has_token;
}

char
get_token( tokenizer_t* tok ) {
    return tok->token;
}

char
get_prev( tokenizer_t* tok ) {
    return tok->prev;
}

char
get_prev_prev( tokenizer_t* tok ) {
    return tok->prev_prev;
}

void
delete_tokenizer( tokenizer_t* tok ) {
    free( tok->stream );
    free( tok );
}

#ifndef __TOMLIBC_TOKENIZER_H__
#define __TOMLIBC_TOKENIZER_H__

#include "models.h"

#include <stdio.h>

#define MAX_NUM_LINES 65536

/*
    Struct `tokenizer` handles the input stream
    by reading and returning tokens for the parser.
    However, this currently returns each character
    as a token. It allows each state of the parser
    to determine if it is an acceptable token, and
    stop parsing as soon as it is not.

    TODO: Refactor to parse tokens instead of characters
*/
typedef struct tokenizer tokenizer_t;
struct tokenizer {
    /* the input stream */
    FILE*   stream;
    /* the location in the input buffer */
    int     cursor;
    /* the last read in token */
    char    token;
    /* the token read in before `token` */
    char    prev;
    /* the token read in before `prev` */
    char    prev_prev;
    /* boolean to indicate if `token` is non-NULL */
    bool    has_token;
    /* keeps track if we are on a newline */
    bool    newline;
    /* line number in the stream */
    int     line;
    /* column number in the stream */
    int     col;
    /* array where index=line and lines[index]=length */
    int     lines[ MAX_NUM_LINES ];
};

/*
    Function `new_tokenizer` allocates memory for the
    tokenizer, assigns some default initial values to
    the various attributes and returns a pointer to the
    newly allocated memory region.
*/
tokenizer_t* new_tokenizer     ( FILE* filename );

/*
    Function `next_token` reads the next character from the
    input stream. It then stores it in the `token` attribute.
    If we have reached the EOF, the `has_token` attribute is
    set to false. This also updates all the attributes of the
    tokenizer appropriately (except `stream`). If we read in
    a token, i.e. we have not reached EOF, returns 1, else
    returns 0.
*/
int     next_token             ( tokenizer_t* tok );

/*
    Function `backtrack` is used to move the `cursor` back in
    the input stream. This allows look-ahead operations to make
    parsing decisions. For example, let's say we want to decide
    whether to parse something as a date or a number. To know if
    there is a `-` a few chars ahead, we call `next_token`.
    However, in the case that there isn't, we need to go back
    to where we were and re-parse it as a number. This function
    takes an argument `count` and backtracks that many characters.
    To maintain the values of `prev` and `prev_prev`, this goes
    back 2 extra characters and calls `next_token` twice to
    re-populate them.
*/
void    backtrack              ( tokenizer_t* tok, int count );

/*
    Function `has_token` returns true if the boolean attribute
    is set to true. This should be used callers to query if
    the tokenizer has a non-EOF token waiting to be parsed.
*/
bool    has_token              ( tokenizer_t* tok );

/*
    Functions `get_token`, `get_prev` and `get_prev_prev`
    returns the `token`, `prev` and `prev_prev` attributes
    held by the tokenizer, respectively. This should be
    used by callers to access the tokens read in by the
    tokenizer.
*/
char    get_token              ( tokenizer_t* tok );
char    get_prev               ( tokenizer_t* tok );
char    get_prev_prev          ( tokenizer_t* tok );

/*
    Function `get_input_size` calculates the size of the
    input stream held by the tokenizer and returns it.
*/
long    get_input_size         ( tokenizer_t* tok );

void    delete_tokenizer       ( tokenizer_t* tok );

#endif

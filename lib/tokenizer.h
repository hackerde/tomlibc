#ifndef __LIB_TOKENIZER_H__
#define __LIB_TOKENIZER_H__

#include "models.h"

typedef struct tokenizer tokenizer_t;
struct tokenizer {
    FILE*   stream;
    int     cursor;
    char    token;
    char    prev;
    char    prev_prev;
    bool    has_token;
    bool    newline;
    int     line;
    int     col;
};

tokenizer_t* new_tokenizer     ( FILE* filename );

size_t  next_token             ( tokenizer_t* tok );
void    backtrack              ( tokenizer_t* tok, int count );

bool    has_token              ( tokenizer_t* tok );
char    get_token              ( tokenizer_t* tok );
char    get_prev               ( tokenizer_t* tok );
char    get_prev_prev          ( tokenizer_t* tok );

#endif

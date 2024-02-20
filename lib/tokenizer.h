#ifndef __LIB_TOKENIZER_H__
#define __LIB_TOKENIZER_H__

#include "models.h"

// NOTE(AJB) The headers in lib/ generally have great hygiene. Only possible problem is not being prefixes with
// something like "toml_". This is (usually) okay as long as these headers aren't publicaly exposed. And.. we all
// get lazy about it at some point anyways (it's annoying).

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

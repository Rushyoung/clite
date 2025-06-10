#ifndef __SCANNER_H__
#define __SCANNER_H__

#include "def.h"
#include "token.h"


struct _scanner{
    size_t size;
    size_t line;
    char*  src;
    char*  cur;
    char*  pre;
    token_t idt;
    token_t prv;
    token_t loc;
};
typedef struct _scanner* scanner;

scanner InitScanner(size_t size, char* src);

token_t idnt(scanner sc, context_t ctx);
token_t prev(scanner sc, context_t ctx);
token_t prst(scanner sc, context_t ctx);

void    next(scanner sc, context_t ctx);


#endif//__SCANNER_H__
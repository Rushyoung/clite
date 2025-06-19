#ifndef __PARSER_H__
#define __PARSER_H__

#include "def.h"
#include "scanner.h"

typedef enum{
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_TERNARY,     // ? : ternary
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_BITWISE,     // & | ^
    PREC_SHIFT,       // << >>
    PREC_TERM,        // + -
    PREC_FACTOR,      // * / %
    PREC_UNARY,       // ! -
    PREC_CALL,        // ()
    PREC_OFFSET,      // []
    PREC_PRIMARY
} PrecLv;

#define ParseFunctionArgs context_t ctx, scanner sc, int can_assign

typedef void (*ParseFn)(ParseFunctionArgs);
typedef struct{
    ParseFn prefix;
    ParseFn infix;
    PrecLv  prec;
} ParseRule;

void BCG_compile(context_t ctx, scanner sc);    // bytecode generation
void AOT_compile(context_t ctx, scanner sc);    // ahead-of-time compilation
void JIT_compile(context_t ctx, scanner sc);    // just-in-time compilation

#endif//__PARSER_H__
#ifndef __PARSER_H__
#define __PARSER_H__

#include "def.h"
#include "scanner.h"

typedef enum{
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_BITWISE,     // & | ^
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_INCREMENT,   // ++ --
    PREC_CALL,        // ()
    PREC_OFFSET,      // []
    PREC_PRIMARY
} PrecLv;

#define ParseFunctionArgs context_t ctx, scanner sc, int can_assign
#define PassFunctionArgs ctx, sc, can_assign

typedef void (*ParseFn)(ParseFunctionArgs);
typedef struct{
    ParseFn prefix;
    ParseFn infix;
    PrecLv  prec;
} ParseRule;

int  parse_expr(context_t ctx, scanner sc, int lev);
void parse_stmt(context_t ctx, scanner sc);
void parse_global(context_t ctx, scanner sc);

void compile(context_t ctx, scanner sc);

#endif//__PARSER_H__
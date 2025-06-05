#ifndef __PARSER_H__
#define __PARSER_H__

#include "def.h"
#include "scanner.h"


int  parse_expr(context_t ctx, scanner sc, int lev);
void parse_stmt(context_t ctx, scanner sc);
void parse_global(context_t ctx, scanner sc);

void compile(context_t ctx, scanner sc);

#endif//__PARSER_H__
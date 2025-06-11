#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "def.h"
#include "scanner.h"
#include "token.h"

void DumpToken(context_t ctx, token_t tk);
void DumpScanner(scanner scanner);
void DumpBtcode(context_t ctx);
void DumpSymbolTable(context_t ctx);

#endif//__DEBUG_H__
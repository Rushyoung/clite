#ifndef __COMPILER_H__
#define __COMPILER_H__

#include "def.h"

void* jitalloc();

void compile(context_t ctx, uint8_t* fun, size_t bt_start, size_t bt_end);


#endif//__COMPILER_H__
#ifndef __COMPILER_H__
#define __COMPILER_H__

#include "def.h"

void* allocate(size_t size);

uint8_t* compile(context_t ctx, size_t bt_start, size_t bt_end);


#endif//__COMPILER_H__
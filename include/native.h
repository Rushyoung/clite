#ifndef __NATIVE_H__
#define __NATIVE_H__

#include <stdint.h>

#include "opcode.h"

#define NativeFunctionArgs uint64_t *bp, uint64_t arity

typedef uint64_t (*NativeFn)(NativeFunctionArgs);

uint64_t lite_open(NativeFunctionArgs);
uint64_t lite_read(NativeFunctionArgs);
uint64_t lite_close(NativeFunctionArgs);
uint64_t lite_printf(NativeFunctionArgs);
uint64_t lite_input(NativeFunctionArgs);
uint64_t lite_malloc(NativeFunctionArgs);
uint64_t lite_free(NativeFunctionArgs);
uint64_t lite_memset(NativeFunctionArgs);
uint64_t lite_memcmp(NativeFunctionArgs);
uint64_t lite_exit(NativeFunctionArgs);
uint64_t lite_time(NativeFunctionArgs);
uint64_t lite_sleep(NativeFunctionArgs);
uint64_t lite_rand(NativeFunctionArgs);

uint64_t lite_debug_sp(NativeFunctionArgs);

uint64_t buildin_list(NativeFunctionArgs);  // just use in bytecode


#endif//__NATIVE_H__
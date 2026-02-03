#ifndef __NATIVE_H__
#define __NATIVE_H__

#include <stdint.h>

#define NativeFunctionArgs uint64_t *bp, uint64_t arity

typedef uint64_t (*NativeFn)(NativeFunctionArgs);

uint64_t lite_open(NativeFunctionArgs);
uint64_t lite_read(NativeFunctionArgs);
uint64_t lite_close(NativeFunctionArgs);
uint64_t lite_printf(NativeFunctionArgs);
uint64_t lite_fgets(NativeFunctionArgs);
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

enum {
    GC_INIT,
    GC_CLEAR,
    GC_REGISTER_PTR,
};
uint64_t builtin_gc(NativeFunctionArgs); // the gc core function
uint64_t builtin_gc_init(NativeFunctionArgs); // use in the runner startup
uint64_t builtin_gc_clear(NativeFunctionArgs); // use in the OP_GC

#endif//__NATIVE_H__
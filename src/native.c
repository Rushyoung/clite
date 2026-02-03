#include "native.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#define assert(cond, msg) \
    if (!(cond)) { \
        fprintf(stderr, msg); \
        exit(EXIT_FAILURE); \
    }


uint64_t lite_open(NativeFunctionArgs) {
    assert(arity == 2, "lite_open requires 2 arguments");
    char *filename = (char *)(bp[0]);
    int flags = (int)(bp[1]);
    int fd = open(filename, flags);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    return fd;
}


uint64_t lite_read(NativeFunctionArgs) {
    assert(arity == 3, "lite_read requires 3 arguments");
    int fd = (int)(bp[0]);
    char *buffer = (char *)(bp[1]);
    uint32_t size = (uint32_t)(bp[2]);
    ssize_t bytes_read = read(fd, buffer, size);
    if (bytes_read < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }
    return bytes_read;
}


uint64_t lite_close(NativeFunctionArgs) {
    assert(arity == 1, "lite_close requires 1 argument");
    int fd = (int)(bp[0]);
    if (close(fd) < 0) {
        perror("close");
        exit(EXIT_FAILURE);
    }
    return 0;
}


uint64_t lite_printf(NativeFunctionArgs) {
    assert(arity >= 1, "lite_printf must have the format string as the first argument");
    assert(arity <= 6, "lite_printf supports up to 6 arguments");
    return printf((char*)bp[0], bp[1], bp[2], bp[3], bp[4], bp[5]);
}


uint64_t lite_fgets(NativeFunctionArgs) {
    assert(arity == 3, "lite_fgets requires 3 arguments");
    char *buffer = (char *)(bp[0]);
    int size = (int)(bp[1]);
    int fd = (int)(bp[2]);
    if(size < 0){
        return 0;
    }
    if(fd == 0) {
        return (uint64_t)fgets(buffer, size, stdin);
    }
    int i = 0;
    char c;
    ssize_t ret;
    while (i < size - 1) {
        ret = read(fd, &c, 1);
        if (ret == 1) {
            buffer[i++] = c;
            if (c == '\n') { // 读到换行符就停止
                break;
            }
        } else if (ret == 0) { // EOF
            if (i == 0) return 0; // 如果还没读到任何东西就 EOF，返回 NULL
            break; // 如果读了一部分遇到 EOF，结束循环返回已读内容
        } else {
            return 0; // 读取错误
        }
    }
    buffer[i] = '\0'; // 补上字符串结束符
    return bp[0];
}


uint64_t lite_malloc(NativeFunctionArgs) {
    assert(arity == 1, "lite_malloc requires 1 argument");
    size_t size = (size_t)(bp[0]);
    void *ptr = malloc(size);
    if (!ptr) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    return (uint64_t)ptr;
}


uint64_t lite_free(NativeFunctionArgs) {
    assert(arity == 1, "lite_free requires 1 argument");
    void *ptr = (void *)(bp[0]);
    free(ptr);
    return 0;
}


uint64_t lite_memset(NativeFunctionArgs) {
    assert(arity == 3, "lite_memset requires 3 arguments");
    void *ptr = (void *)(bp[0]);
    int value = (int)(bp[1]);
    size_t size = (size_t)(bp[2]);
    memset(ptr, value, size);
    return 0;
}


uint64_t lite_memcmp(NativeFunctionArgs) {
    assert(arity == 3, "lite_memcmp requires 3 arguments");
    void *ptr1 = (void *)(bp[0]);
    void *ptr2 = (void *)(bp[1]);
    size_t size = (size_t)(bp[2]);
    return memcmp(ptr1, ptr2, size);
}


uint64_t lite_exit(NativeFunctionArgs) {
    assert(arity == 1, "lite_exit requires 1 argument");
    int status = (int)(bp[0]);
    exit(status);
}


uint64_t lite_time(NativeFunctionArgs) {
    assert(arity == 1, "lite_time requires 1 argument");
    assert(bp[0] == 0, "lite_time does not support non-zero argument");
    return (uint64_t)time(NULL); // return current time in seconds
}


uint64_t lite_sleep(NativeFunctionArgs) {
    assert(arity == 1, "lite_sleep requires 1 argument");
    int seconds = (int)(bp[0]);
    if (seconds < 0) {
        fprintf(stderr, "lite_sleep requires non-negative argument\n");
        exit(EXIT_FAILURE);
    }
    sleep(seconds);
    return 0;
}


uint64_t lite_rand(NativeFunctionArgs) {
    if (arity != 0) {
        fprintf(stderr, "lite_rand requires no arguments\n");
        exit(EXIT_FAILURE);
    }
    return (uint64_t)(rand());
}


uint64_t lite_debug_sp(NativeFunctionArgs) {
    assert(arity == 0, "lite_debug_sp requires 0 arguments");
    fprintf(stderr, "Debug: Stack Pointer = %p\n", bp);
    return 0;
}


uint64_t buildin_list(NativeFunctionArgs) {
    uint64_t *list = malloc(arity * sizeof(uint64_t));
    if (!list) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    if(builtin_gc(list, GC_REGISTER_PTR) == 1){ // register the list for GC
        builtin_gc(bp, GC_CLEAR); // perform GC if registration fails
        assert(builtin_gc(list, GC_REGISTER_PTR) == 0, "Failed to register pointer after GC");
    }
    for (uint64_t i = 0; i < arity; i++) {
        list[i] = bp[i];
    }
    return (uint64_t)list; // return pointer to the list
}


uint64_t builtin_gc(NativeFunctionArgs) {
    static uint64_t* stk = NULL;
    static uint64_t** ptrs = NULL;
    static size_t reg = 0;
    switch(arity){
        case GC_INIT: // init all static variables
            assert(stk == NULL, "GC already initialized");
            stk = bp;
            ptrs = malloc(1024 * sizeof(uint64_t*));
            for(size_t i = 0; i < 1024; i++){
                ptrs[i] = NULL;
            }
            reg = 0;
            assert(ptrs != NULL, "Failed to allocate memory for GC pointers");
            return 0;
        case GC_CLEAR: { // perform garbage collection
            assert(stk != NULL, "GC has not initialized");
            size_t new_reg = 0;
            for(size_t idx = 0; idx < reg; idx++){
                uint64_t* ptr = ptrs[idx];
                if(ptr == NULL){
                    continue; // skip null pointers
                }
                int found = 0;
                for(uint64_t* p = stk; p < bp; p++){
                    if(*p == (uint64_t)ptr){
                        found = 1;
                        break;
                    }
                }
                if(!found){
                    free(ptr);
                    ptrs[idx] = NULL; // clear the pointer after freeing
                } else {
                    ptrs[new_reg++] = ptr; // keep the pointer if it's still referenced
                }
            }
            reg = new_reg; // update the register count after GC
            return 0;
        }
        case GC_REGISTER_PTR: // register a pointer for GC
            assert(stk != NULL, "GC not initialized");
            if(reg >= 1024){
                return 1;
            }
            ptrs[reg] = bp;
            reg++;
            return 0;
    }
}


uint64_t builtin_gc_init(NativeFunctionArgs) {
    assert(arity == -1, "builtin_gc_init can only be used during runner startup, and should be called by hand.");
    builtin_gc(bp, GC_INIT);
    return 0;
}


uint64_t builtin_gc_clear(NativeFunctionArgs) {
    assert(arity == -1, "builtin_gc_clear can only be called by hard-coded in the bytecode.");
    static uint64_t called_time = 0;
    called_time++;
    if(called_time >= 100){
        called_time = 0;
        builtin_gc(bp, GC_CLEAR); // perform GC every 100 calls
    }
    return 0;
}
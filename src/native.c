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
    return printf(bp[0], bp[1], bp[2], bp[3], bp[4], bp[5]);
}


uint64_t lite_input(NativeFunctionArgs) {
    assert(arity == 1, "lite_input requires 1 argument");
    char *buffer = (char *)(bp[0]);
    ssize_t bytes_read = read(STDIN_FILENO, buffer, 1024);
    if (bytes_read < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }
    buffer[bytes_read - 1] = '\0'; // null-terminate the string
    return (uint64_t)bytes_read; // return number of bytes read
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
    for (uint64_t i = 0; i < arity; i++) {
        list[i] = bp[i];
    }
    return (uint64_t)list; // return pointer to the list
}
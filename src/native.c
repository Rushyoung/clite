#include "native.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "opcode.h"


uint64_t lite_open(NativeFunctionArgs) {
    if (arity != 1) {
        fprintf(stderr, "lite_open requires 1 argument\n");
        exit(EXIT_FAILURE);
    }
    char *filename = (char *)(bp[0]);
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    return fd;
}


uint64_t lite_read(NativeFunctionArgs) {
    if (arity != 2) {
        fprintf(stderr, "lite_read requires 2 arguments\n");
        exit(EXIT_FAILURE);
    }
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
    if (arity != 1) {
        fprintf(stderr, "lite_close requires 1 argument\n");
        exit(EXIT_FAILURE);
    }
    int fd = (int)(bp[0]);
    if (close(fd) < 0) {
        perror("close");
        exit(EXIT_FAILURE);
    }
    return 0;
}


uint64_t lite_printf(NativeFunctionArgs) {
    if(arity > 6){
        fprintf(stderr, "lite_printf supports up to 6 arguments\n");
        exit(EXIT_FAILURE);
    }
    printf(bp[0], bp[1], bp[2], bp[3], bp[4], bp[5]);
    return 0;
}


uint64_t lite_input(NativeFunctionArgs) {
    if (arity != 1) {
        fprintf(stderr, "lite_input requires 1 argument\n");
        exit(EXIT_FAILURE);
    }
    char *buffer = (char *)(bp[0]);
    ssize_t bytes_read = read(STDIN_FILENO, buffer, 1024);
    if (bytes_read < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }
    buffer[bytes_read] = '\0'; // null-terminate the string
    return (uint64_t)bytes_read; // return number of bytes read
}


uint64_t lite_malloc(NativeFunctionArgs) {
    if (arity != 1) {
        fprintf(stderr, "lite_malloc requires 1 argument\n");
        exit(EXIT_FAILURE);
    }
    size_t size = (size_t)(bp[0]);
    void *ptr = malloc(size);
    if (!ptr) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    return (uint64_t)ptr;
}


uint64_t lite_free(NativeFunctionArgs) {
    if (arity != 1) {
        fprintf(stderr, "lite_free requires 1 argument\n");
        exit(EXIT_FAILURE);
    }
    void *ptr = (void *)(bp[0]);
    free(ptr);
    return 0;
}


uint64_t lite_memset(NativeFunctionArgs) {
    if (arity != 3) {
        fprintf(stderr, "lite_memset requires 3 arguments\n");
        exit(EXIT_FAILURE);
    }
    void *ptr = (void *)(bp[0]);
    int value = (int)(bp[1]);
    size_t size = (size_t)(bp[2]);
    memset(ptr, value, size);
    return 0;
}


uint64_t lite_memcmp(NativeFunctionArgs) {
    if (arity != 3) {
        fprintf(stderr, "lite_memcmp requires 3 arguments\n");
        exit(EXIT_FAILURE);
    }
    void *ptr1 = (void *)(bp[0]);
    void *ptr2 = (void *)(bp[1]);
    size_t size = (size_t)(bp[2]);
    return memcmp(ptr1, ptr2, size);
}


uint64_t lite_exit(NativeFunctionArgs) {
    if (arity != 1) {
        fprintf(stderr, "lite_exit requires 1 argument\n");
        exit(EXIT_FAILURE);
    }
    int status = (int)(bp[0]);
    exit(status);
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


int is_native(uint64_t func) {
    return func == lite_close ||
           func == lite_open ||
           func == lite_read ||
           func == lite_printf ||
           func == lite_input ||
           func == buildin_list ||
           func == lite_malloc ||
           func == lite_free ||
           func == lite_memset ||
           func == lite_memcmp ||
           func == lite_exit;
}
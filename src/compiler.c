#include "compiler.h"

#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#endif

void* allocate(size_t size) {
    #ifdef _WIN32
    void* mem = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!mem) { fprintf(stderr, "VirtualAlloc failed\n"); exit(1); }
#else
    void* mem = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) { perror("mmap"); exit(1); }
#endif
    return mem;
}


void* compile(context_t ctx, size_t bt_start, size_t bt_end) {
    if (bt_start >= bt_end) {
        fprintf(stderr, "Invalid bytecode range: %zu to %zu\n", bt_start, bt_end);
        return NULL;
    }

    void* mem = allocate(4096); // Allocate 4096 bytes for the compiled code
    return mem; // Return the allocated memory pointer
}
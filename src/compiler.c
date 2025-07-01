#include "compiler.h"

#include "opcode.h"

#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#endif

#define emit_a(x) (*jit++ = (x)) // emit assembler instruction
#define emit_i(x) ({*(uint64_t*)jit = (x); jit += 8;}) // emit immediate value

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


uint8_t* compile(context_t ctx, size_t bt_start, size_t bt_end) {
    if (bt_start >= bt_end) {
        fprintf(stderr, "Invalid bytecode range: %zu to %zu\n", bt_start, bt_end);
        return NULL;
    }

    uint8_t* jit = allocate(4096); // Allocate 4096 bytes for the compiled code
    uint8_t* fun = jit; // Save the start of the allocated memory
    uint64_t* pc = ctx->btcode + bt_start;
    uint64_t  ip = 0;
    uint64_t* end = ctx->btcode + bt_end;
    // 能用的寄存器
    // RAX, RBX, RCX, RDX, RSI, RDI, R8, R9, R10, R11, R12, R13, R14, R15
    // RSI 是栈顶sp，RDI 是基址bp
    for(ip = *pc; pc < end && ip != 0; ip = *pc) {
        pc++;
        switch (ip) {
            case OP_FUNC:
                emit_a(0x48); emit_a(0x89); emit_a(0xCF); // mov RDI, RCX; RDI 是基址bp
                emit_a(0x48); emit_a(0x89); emit_a(0xFE); // mov RSI, RDI; RSI 是栈顶sp
                emit_a(0x48); emit_a(0x8D); emit_a(0x34); emit_a(0xD6); // lea RSI, [RSI + RDX * 8]; RDX 是参数个数
                emit_a(0x48); emit_a(0xB8); emit_i(0);    // mov RAX, 0x00; RAX 是返回值
                break;
            case OP_G_LOC:
                emit_a(0x48); emit_a(0xBB); emit_i(*pc);                // mov RBX, immediate value
                emit_a(0x48); emit_a(0x8B); emit_a(0x04); emit_a(0xDF); // mov RAX, [RDI + RBX * 8]
                pc++;
                break;
            case OP_IMM:
                emit_a(0x48); emit_a(0xB8); emit_i(*pc); // mov RAX, immediate value
                pc++;
                break;
            case OP_PUSH:
                emit_a(0x48); emit_a(0x89); emit_a(0x06);               // mov [RSI], RAX; 将 RAX 的值压栈
                emit_a(0x48); emit_a(0x83); emit_a(0xC6); emit_a(0x08); // add RSI, 8; 栈顶指针加8
                break;
            case OP_ADD:
                emit_a(0x48); emit_a(0x8B); emit_a(0x5E); emit_a(0xF8); // mov RBX, [RSI - 8]
                emit_a(0x48); emit_a(0x03); emit_a(0xC3);               // add RAX, RBX
                emit_a(0x48); emit_a(0x83); emit_a(0xEE); emit_a(0x08); // sub RSI, 8; 栈顶指针减8
                break;
            case OP_SUB:
                emit_a(0x48); emit_a(0x8B); emit_a(0x5E); emit_a(0xF8); // mov RBX, [RSI - 8]
                emit_a(0x48); emit_a(0x29); emit_a(0xC3);               // sub RAX, RBX
                emit_a(0x48); emit_a(0x83); emit_a(0xEE); emit_a(0x08); // sub RSI, 8; 栈顶指针减8
                break;
            case OP_MUL:
                emit_a(0x48); emit_a(0x8B); emit_a(0x5E); emit_a(0xF8); // mov RBX, [RSI - 8]
                emit_a(0x48); emit_a(0x0F); emit_a(0xAF); emit_a(0xC3); // imul RAX, RBX
                emit_a(0x48); emit_a(0x83); emit_a(0xEE); emit_a(0x08); // sub RSI, 8; 栈顶指针减8
                break;
            case OP_RET:
                emit_a(0xC3); // ret
                break;
            default:
                fprintf(stderr, "Unknown opcode: %llu\n", ip);
                exit(1);
        }
    }
    return fun; // Return the allocated memory pointer
}
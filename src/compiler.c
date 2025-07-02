#include "compiler.h"

#include "opcode.h"
#include "native.h"

#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#endif

#define emit_a(x) (*jit++ = (x)) // emit assembler instruction
#define emit_e(x) ({*(uint32_t*)jit = (x); jit += 4;}) // emit immediate value (32-bit)
#define emit_i(x) ({*(uint64_t*)jit = (x); jit += 8;}) // emit immediate value
#define emit_v(x) ({\
    uint16_t in = (uint16_t)(jit - fun); \
    uint16_t to = (uint16_t)((x) - bt_start); \
    patch_in[patch_count] = in; patch_to[patch_count] = to; ; \
    patch_count++; jit += 4; \
})

void* jitalloc() {
    #ifdef _WIN32
    void* mem = VirtualAlloc(NULL, 4196, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!mem) { fprintf(stderr, "VirtualAlloc failed\n"); exit(1); }
#else
    void* mem = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) { perror("mmap"); exit(1); }
#endif
    return mem;
}


void compile(context_t ctx, uint8_t* fun, size_t bt_start, size_t bt_end) {
    if (bt_start >= bt_end) {
        fprintf(stderr, "Invalid bytecode range: %zu to %zu\n", bt_start, bt_end);
        exit(EXIT_FAILURE);
    }
    uint8_t*  jit = fun; // Save the start of the allocated memory
    uint64_t* pc = ctx->btcode + bt_start;
    uint64_t  ip = 0;
    uint64_t* end = ctx->btcode + bt_end;
    uint16_t  bt2jit[4096] = {};   // bt2jit[id] = val， 
    // 意味着这个函数第id个位置的字节码，在jit的第val个位置
    uint16_t  patch_in[4096] = {}; // patch_in[id] = in
    uint16_t  patch_to[4096] = {}; // patch_to[id] = goal，代表在in个字节处，应该填入一个64位地址
    size_t    patch_count = 0;
    for(ip = *pc; pc < end && ip != 0; ip = *pc) {
        bt2jit[pc - ctx->btcode - bt_start] = (uint16_t)(jit - fun);
        printf("bt[%llu] = bt2jit[%llu] = %u\n", (unsigned long long)(pc - ctx->btcode), (unsigned long long)(pc - ctx->btcode - bt_start), (unsigned)bt2jit[pc - ctx->btcode - bt_start]);
        pc++;
        if(jit - fun >= 4096) {
            fprintf(stderr, "JIT compilation buffer overflow\n");
            exit(1);
        }
        switch (ip) {
            case OP_FUNC:
                emit_a(0x48); emit_a(0x89); emit_a(0xCF); // mov RDI, RCX; RDI 是基址bp
                emit_a(0x48); emit_a(0x89); emit_a(0xFE); // mov RSI, RDI; RSI 是栈顶sp
                emit_a(0x48); emit_a(0x8D); emit_a(0x34); emit_a(0xD6); // lea RSI, [RSI + RDX * 8]; RDX 是参数个数
                emit_a(0x48); emit_a(0xB8); emit_i(0);    // mov RAX, 0x00; RAX 常用寄存器
                break;
            case OP_G_GLO:
                emit_a(0x48); emit_a(0xB8); emit_i(&(ctx->sym[*pc].val)); // mov RAX, global address
                emit_a(0x48); emit_a(0x8B); emit_a(0x00);                 // mov RAX, [RAX]
                pc++;
                break;
            case OP_S_GLO:
                emit_a(0x48); emit_a(0xBB); emit_i(&(ctx->sym[*pc].val)); // mov RBX, global address
                emit_a(0x48); emit_a(0x89); emit_a(0x03);                 // mov [RBX], RAX
                pc++;
                break;
            case OP_G_LOC:
                emit_a(0x48); emit_a(0xBB); emit_i(*pc);                // mov RBX, immediate value
                emit_a(0x48); emit_a(0x8B); emit_a(0x04); emit_a(0xDF); // mov RAX, [RDI + RBX * 8]
                pc++;
                break;
            case OP_S_LOC:
                emit_a(0x48); emit_a(0xBB); emit_i(*pc);                // mov RBX, immediate value
                emit_a(0x48); emit_a(0x89); emit_a(0x04); emit_a(0xDF);
                pc++;
                break;
            case OP_JMP:
                emit_a(0xE9); emit_v(*pc); // jmp address; 跳转到地址
                pc++;
                break;
            case OP_JZ:
                printf("in OP_JZ\n"); // 强制跳转10字节
                emit_a(0x48); emit_a(0x85); emit_a(0xC0); // test RAX, RAX; 检查 RAX 是否为 0
                emit_a(0x0F); emit_a(0x84); emit_v(*pc);  // jz RBX; 如果 RAX 为 0，则跳转到 RBX
                pc++;
                break;
            case OP_IMM:
                emit_a(0x48); emit_a(0xB8); emit_i(*pc); // mov RAX, immediate value
                pc++;
                break;
            case OP_STR:
                emit_a(0x48); emit_a(0xBB); emit_i(ctx->heap); // mov RBX, heap base address
                emit_a(0x48); emit_a(0x03); emit_a(0xC3);      // add RAX, RBX; 将 RAX 的值加上堆基址
                break;
            case OP_SAD:
            case OP_PUSH:
                emit_a(0x48); emit_a(0x89); emit_a(0x06);               // mov [RSI], RAX; 将 RAX 的值压栈
                emit_a(0x48); emit_a(0x83); emit_a(0xC6); emit_a(0x08); // add RSI, 8; 栈顶指针加8
                break;
            case OP_ADD:
                emit_a(0x48); emit_a(0x83); emit_a(0xEE); emit_a(0x08); // sub RSI, 8; 栈顶指针减8
                emit_a(0x48); emit_a(0x8B); emit_a(0x1E);               // mov RBX, [RSI]
                emit_a(0x48); emit_a(0x03); emit_a(0xC3);               // add RAX, RBX
                break;
            case OP_SUB:
                emit_a(0x48); emit_a(0x83); emit_a(0xEE); emit_a(0x08); // sub RSI, 8; 栈顶指针减8
                emit_a(0x48); emit_a(0x8B); emit_a(0x1E);               // mov RBX, [RSI]
                emit_a(0x48); emit_a(0x29); emit_a(0xC3);               // sub RBX, RAX;
                emit_a(0x48); emit_a(0x89); emit_a(0xD8);               // mov RAX, RBX;
                break;
            case OP_MUL:
                emit_a(0x48); emit_a(0x83); emit_a(0xEE); emit_a(0x08); // sub RSI, 8; 栈顶指针减8
                emit_a(0x48); emit_a(0x8B); emit_a(0x1E);               // mov RBX, [RSI]
                emit_a(0x48); emit_a(0x0F); emit_a(0xAF); emit_a(0xC3); // imul RAX, RBX
                break;
            case OP_DIV:
                emit_a(0x48); emit_a(0x83); emit_a(0xEE); emit_a(0x08); // sub RSI, 8; 栈顶指针减8
                emit_a(0x48); emit_a(0x89); emit_a(0xC3); // mov RBX, RAX
                emit_a(0x48); emit_a(0x8B); emit_a(0x06); // mov RAX, [RSI]
                emit_a(0x48); emit_a(0x99);  // cqo // 扩展 RAX 到 RDX:RAX
                emit_a(0x48); emit_a(0xF7); emit_a(0xFB); // idiv RBX; 除法，结果在 RAX 中
                break;
            case OP_MOD:
                emit_a(0x48); emit_a(0x83); emit_a(0xEE); emit_a(0x08); // sub RSI, 8; 栈顶指针减8
                emit_a(0x48); emit_a(0x89); emit_a(0xC3); // mov RBX, RAX
                emit_a(0x48); emit_a(0x8B); emit_a(0x06); // mov RAX, [RSI]
                emit_a(0x48); emit_a(0x99);  // cqo // 扩展 RAX 到 RDX:RAX
                emit_a(0x48); emit_a(0xF7); emit_a(0xFB); // idiv RBX; 除法，结果在 RAX 中
                emit_a(0x48); emit_a(0x89); emit_a(0xD0); // mov RAX, RDX; 余数在 RDX 中
                break;
            case OP_OR:
                emit_a(0x48); emit_a(0x83); emit_a(0xEE); emit_a(0x08); // sub RSI, 8; 栈顶指针减8
                emit_a(0x48); emit_a(0x8B); emit_a(0x1E);               // mov RBX, [RSI]
                emit_a(0x48); emit_a(0x09); emit_a(0xC3);               // or RAX, RBX
                break;
            case OP_XOR:
                emit_a(0x48); emit_a(0x83); emit_a(0xEE); emit_a(0x08); // sub RSI, 8; 栈顶指针减8
                emit_a(0x48); emit_a(0x8B); emit_a(0x1E);               // mov RBX, [RSI]
                emit_a(0x48); emit_a(0x31); emit_a(0xC3);               // xor RAX, RBX
                break;
            case OP_AND:
                emit_a(0x48); emit_a(0x83); emit_a(0xEE); emit_a(0x08); // sub RSI, 8; 栈顶指针减8
                emit_a(0x48); emit_a(0x8B); emit_a(0x1E);               // mov RBX, [RSI]
                emit_a(0x48); emit_a(0x21); emit_a(0xC3);               // and RAX, RBX
                break;
            case OP_EQU:
                emit_a(0x48); emit_a(0x83); emit_a(0xEE); emit_a(0x08); // sub RSI, 8; 栈顶指针减8
                emit_a(0x48); emit_a(0x8B); emit_a(0x1E);               // mov RBX, [RSI]
                emit_a(0x48); emit_a(0x39); emit_a(0xD8);               // cmp RAX, RBX
                emit_a(0x0F); emit_a(0x94); emit_a(0xC0);               // sete AL; 如果相等, 设置 AL 为 1
                emit_a(0x0F); emit_a(0xB6); emit_a(0xC0); // movzx RAX, AL; 扩展 AL 到 RAX
                break;
            case OP_GRT:
                emit_a(0x48); emit_a(0x83); emit_a(0xEE); emit_a(0x08); // sub RSI, 8; 栈顶指针减8
                emit_a(0x48); emit_a(0x8B); emit_a(0x1E);               // mov RBX, [RSI]
                emit_a(0x48); emit_a(0x39); emit_a(0xD8);               // cmp RAX, RBX
                emit_a(0x0F); emit_a(0x9C); emit_a(0xC0);               // setl AL; 如果 RBX > RAX, 设置 AL 为 1
                emit_a(0x0F); emit_a(0xB6); emit_a(0xC0);               // movzx RAX, AL; 扩展 AL 到 RAX
                break;
            case OP_LES:
                emit_a(0x48); emit_a(0x83); emit_a(0xEE); emit_a(0x08); // sub RSI, 8; 栈顶指针减8
                emit_a(0x48); emit_a(0x8B); emit_a(0x1E);               // mov RBX, [RSI]
                emit_a(0x48); emit_a(0x39); emit_a(0xD8);               // cmp RAX, RBX
                emit_a(0x0F); emit_a(0x9F); emit_a(0xC0);               // setg AL; 如果 RBX < RAX, 设置 AL 为 1
                emit_a(0x0F); emit_a(0xB6); emit_a(0xC0);               // movzx RAX, AL; 扩展 AL 到 RAX
                break;
            case OP_SHL: // @bug: 使用 CL 寄存器作为移位量
                emit_a(0x48); emit_a(0x83); emit_a(0xEE); emit_a(0x08); // sub RSI, 8; 栈顶指针减8
                emit_a(0x48); emit_a(0x8B); emit_a(0x1E);               // mov RBX, [RSI]
                emit_a(0x48); emit_a(0xD3); emit_a(0xE0);               // shl RAX, CL; 使用 CL 寄存器作为移位量
                break;
            case OP_SHR: // @bug: 使用 CL 寄存器作为移位量
                emit_a(0x48); emit_a(0x83); emit_a(0xEE); emit_a(0x08); // sub RSI, 8; 栈顶指针减8
                emit_a(0x48); emit_a(0x8B); emit_a(0x1E);               // mov RBX, [RSI]
                emit_a(0x48); emit_a(0xD3); emit_a(0xE8);               // shr RAX, CL; 使用 CL 寄存器作为移位量
                break;
            case OP_NOT:
                emit_a(0x48); emit_a(0x85); emit_a(0xC0); // test RAX, RAX
                emit_a(0x0F); emit_a(0x94); emit_a(0xC0); // sete AL
                emit_a(0x0F); emit_a(0xB6); emit_a(0xC0); // movzx RAX, AL
                break;
            case OP_CALL:
                emit_a(0x48); emit_a(0x83); emit_a(0xEC); emit_a(0x28); // sub rsp, 40 // 对齐40字节栈空间
                emit_a(0x57); // push RDI // 保存基址bp
                emit_a(0x56); // push RSI // 保存栈顶sp
                emit_a(0x49); emit_a(0xBA); emit_i(*pc); // mov R10, argc(imm64)
                emit_a(0x48); emit_a(0x89); emit_a(0xF1); // mov RCX, RSI
                emit_a(0x4C); emit_a(0x89); emit_a(0xD2); // mov RDX, R10
                emit_a(0x48); emit_a(0xC1); emit_a(0xE2); emit_a(0x03); // shl RDX, 3
                emit_a(0x48); emit_a(0x29); emit_a(0xD1); // sub RCX, RDX
                emit_a(0x48); emit_a(0x8B); emit_a(0x41); emit_a(0xF8); // mov RAX, [RCX-8]
                emit_a(0x4C); emit_a(0x89); emit_a(0xD2); // mov RDX, R10
                emit_a(0xFF); emit_a(0xD0); // call RAX
                emit_a(0x5E); // pop RSI // 恢复栈顶sp
                emit_a(0x5F); // pop RDI // 恢复基址bp
                emit_a(0x48); emit_a(0x83); emit_a(0xC4); emit_a(0x28); // add rsp, 40
                emit_a(0x48); emit_a(0xC7); emit_a(0xC2); emit_e((*pc + 1) * 8); // mov RDX, 回退量 = (argc+1)*8
                emit_a(0x48); emit_a(0x29); emit_a(0xD6); // sub RSI, RDX; 恢复虚拟栈指针
                pc++;
                break;
            case OP_RET:
                emit_a(0xC3); // ret
                break;
            default:
                fprintf(stderr, "Unknown opcode: %llu in %d\n", ip, (int)(pc - ctx->btcode - 1));
                exit(1);
        }
    }
    for(int cur = 0; cur < patch_count; cur++) {
        // target = bt2jit[patch_to[cur]];
        // current = patch_in[cur];
        // len = patch_len[cur];
        int32_t jmp = bt2jit[patch_to[cur]] - (patch_in[cur] + 4);
        uint8_t* patch_addr = fun + patch_in[cur];
        *(uint32_t*)patch_addr = jmp; // 填充跳转地址
        printf("%llu want to jmp to %u->%u, need %d bytes\n patch", 
               (unsigned long long)(patch_in[cur] + 4), (unsigned)patch_to[cur],
               (unsigned)bt2jit[patch_to[cur]], jmp);
        printf("\n");
    }
}
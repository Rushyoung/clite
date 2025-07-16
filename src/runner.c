#include "runner.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "def.h"
#include "opcode.h"
#include "native.h"


void run(context_t ctx){
    uint64_t* stk = NULL; // stack pointer
    uint64_t* bp = NULL;  // base pointer
    uint64_t* sp = NULL;  // stack pointer
    uint64_t* pc = ctx->btcode; // program counter
    uint64_t  ip = 0;     // instruction pointer
    uint64_t  ax = 0;     
    stk = bp = sp = (uint64_t*)malloc(8192 * sizeof(uint64_t)); // allocate stack memory
    if(!stk){
        fprintf(stderr, "Failed to allocate memory for stack\n");
        exit(EXIT_FAILURE);
    }
    for(ip = *pc; ip != 0; ip = *pc){
        pc++;
        if(__args__.debug){
            printf("%2llu OP=%2llu, sp = %2lld, ax = %lld\n", pc - ctx->btcode - 1, ip, sp - stk, ax);
        }
        switch(ip){
            case OP_G_GLO:
                ax = ctx->sym[*pc].val; // get global variable value
                pc++;
                break;
            case OP_S_GLO:
                ctx->sym[*pc].val = ax; // set global variable value
                pc++;
                break;
            case OP_G_LOC:
                ax = *(bp + *pc);
                pc++;
                break;
            case OP_S_LOC:
                *(bp + *pc) = ax;
                pc++;
                break;
            case OP_IMM:
                ax = *pc;
                pc++;
                break;
            case OP_LOOP: // 循环结束标记，什么都不做
            case OP_FUNC: // 函数入口标记，什么都不做
                break;
            case OP_CALL:
                ax = *(sp - *pc - 1);                               // 获取函数地址
                *(sp - *pc - 2) = (uint64_t)(bp - stk);             // 保存基指针位置
                *(sp - *pc - 1) = (uint64_t)(pc - ctx->btcode + 1); // 保存返回地址
                bp = sp - *pc; // 更新基指针
                if(ax < 65535 && *(ctx->btcode + ax) == OP_FUNC){
                    pc = ctx->btcode + ax + 1;
                    break;
                } else {
                    NativeFn fn = (NativeFn)ax;
                    ax = fn(bp, *pc);
                }
            case OP_RET:
                if(bp == stk){
                    free(stk);
                    exit((uint8_t)ax);
                }
                pc = ctx->btcode + *(bp - 1); // 恢复返回地址
                sp = bp - 2; // 恢复栈指针
                bp = stk + *(bp - 2); // 恢复基指针
                break;
            case OP_JMP:
                pc = ctx->btcode + *pc; // 跳转到指定地址
                break;
            case OP_JZ:
                if(ax){
                    pc++;
                } else {
                    pc = ctx->btcode + *pc;
                }
                break;
            case OP_JEND:{
                uint64_t flag = *pc;
                pc++;
                while(*pc != OP_LOOP){
                    if(OP_G_GLO <= *pc && *pc <= OP_CALL){
                        pc++;
                    }
                    pc++;
                }
                if(flag){
                    uint64_t goal = *(pc - 1);
                    pc = ctx->btcode + goal;
                }
                break;
            }
            case OP_PUSH:
                *sp = ax;
                sp++;
                break;
            case OP_G_OFF:{
                uint64_t offset = ax;
                sp--;
                void *addr = (void*)(*sp);
                if(ctx->heap <= addr && addr < ctx->heap + ctx->heap_cur){
                    char chr = *(char*)(addr + offset);
                    ax = (uint64_t)chr;
                } else {
                    ax = *(uint64_t*)(addr + offset * sizeof(uint64_t));
                }
                break;
            }
            case OP_S_OFF:{
                sp--;
                uint64_t offset = *sp;
                sp--;
                void *addr = (void*)(*sp);
                if(ctx->heap <= addr && addr < ctx->heap + ctx->heap_cur){
                    *(char*)(addr + offset) = (char)ax;
                } else {
                    *(uint64_t*)(addr + offset * sizeof(uint64_t)) = ax;
                }
                break;
            }
            case OP_OR:
                sp--;
                ax = *sp | ax;
                break;
            case OP_XOR:
                sp--;
                ax = *sp ^ ax;
                break;
            case OP_AND:
                sp--;
                ax = *sp & ax;
                break;
            case OP_EQU:
                sp--;
                ax = (*sp == ax);
                break;
            case OP_GRT:
                sp--;
                ax = ((int64_t)*sp > (int64_t)ax);
                break;
            case OP_LES:
                sp--;
                ax = ((int64_t)*sp < (int64_t)ax);
                break;
            case OP_SHL:
                sp--;
                ax = (*sp << ax);
                break;
            case OP_SHR:
                sp--;
                ax = (*sp >> ax);
                break;
            case OP_ADD:
                sp--;
                ax = (*sp + ax);
                break;
            case OP_SUB:
                sp--;
                ax = (*sp - ax);
                break;
            case OP_MUL:
                sp--;
                ax = (*sp * ax);
                break;
            case OP_DIV:
                if(ax == 0){
                    fprintf(stderr, "Division by zero\n");
                    free(stk);
                    exit(EXIT_FAILURE);
                }
                sp--;
                ax = (*sp / ax);
                break;
            case OP_MOD:
                if(ax == 0){
                    fprintf(stderr, "Division by zero\n");
                    free(stk);
                    exit(EXIT_FAILURE);
                }
                sp--;
                ax = (*sp % ax);
                break;
            case OP_NOT:
                ax = !ax;
                break;
            case OP_STR:
                ax = ctx->heap + ax; // convert to string address
                break;
            default:
                fprintf(stderr, "Unknown opcode: %llu in %d\n", ip, (int)(pc - ctx->btcode - 1));
                free(stk);
                exit(EXIT_FAILURE);
        }
    }
}

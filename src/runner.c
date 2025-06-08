#include "runner.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "def.h"
#include "opcode.h"


int run(context_t ctx){
    uint64_t* stk = NULL;
    uint64_t* bp = NULL;
    uint64_t* sp = NULL;
    uint64_t* pc = NULL;
    uint64_t  ip = 0;
    uint64_t  ax = 0;
    stk = bp = sp = (uint64_t*)malloc(8192 * sizeof(uint64_t)); // allocate stack memory
    if(!stk){
        fprintf(stderr, "Failed to allocate memory for stack\n");
        return -1;
    }
    if(ctx->sym[ctx->main_id].val == 114514){ // check if main function is defined
        fprintf(stderr, "Main function is not defined\n");
        free(stk);
        return -1;
    }
    pc = ctx->btcode + ctx->sym[ctx->main_id].val; // set program counter to main function
    if(!pc){
        fprintf(stderr, "Main function not found\n");
        free(stk);
        return -1;
    }
    for(ip = *pc; ip != 0; ip = *pc){
        pc++;
        printf("%2llu OP=%2llu, sp = %2d, ax = %d\n", pc - ctx->btcode - 1, ip, sp - stk, ax);
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
                sp = bp + *pc + 2;      // 用意不明
                pc++;
                break;
            case OP_IMM:
                ax = *pc;
                pc++;
                break;
            case OP_JMP:
                pc = stk + *pc;
                break;
            case OP_SAD:
                // 保存当前栈地址到栈顶
                *sp = (uint64_t)(sp - stk);
                sp++;
                *sp = (uint64_t)(bp - stk); // 保存当前基指针位置
                sp++;
                *sp = 0; // 占位符，后续覆盖为函数返回地址
                sp++;
                bp = sp; // 更新基指针
                break;
            case OP_CALL:
                // 保存当前基指针和栈指针
                printf("call to function at %llu\n", ax);
                printf("bp will be set to %llu\n", bp - stk);
                *(bp - 1) = (uint64_t)(pc - ctx->btcode + 1); // 保存返回地址
                printf("Function need return to %llu\n", *(bp - 1));
                pc = ctx->btcode + ax; // 跳转到函数地址
                break;
            case OP_RET:
                if(bp == stk){
                    fprintf(stderr, "Return from main function\n");
                    free(stk);
                    return ax;
                }
                printf("return from function, ax = %d\n", ax);
                pc = ctx->btcode + *(bp - 1); // 恢复返回地址
                sp = bp - 3; // 恢复栈指针
                bp = stk + *(bp - 2); // 恢复基指针
                break;
            case OP_JZ:
                if(ax){
                    pc++;
                } else {
                    pc = stk + *pc;
                }
                break;
            case OP_PUSH:
                printf("push %d to stack %d\n", ax, sp - stk);
                *sp = ax;
                sp++;
                break;
            case OP_OR:
                ax = *sp | ax;
                sp--;
                break;
            case OP_XOR:
                ax = *sp ^ ax;
                sp--;
                break;
            case OP_AND:
                ax = *sp & ax;
                sp--;
                break;
            case OP_EQU:
                sp--;
                ax = (*sp == ax);
                break;
            case OP_GRT:
                sp--;
                ax = (*sp > ax);
                break;
            case OP_LES:
                sp--;
                ax = (*sp < ax);
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
                    return -1;
                }
                sp--;
                ax = (*sp / ax);
                break;
            case OP_MOD:
                if(ax == 0){
                    fprintf(stderr, "Division by zero\n");
                    free(stk);
                    return -1;
                }
                sp--;
                ax = (*sp % ax);
                break;
            case OP_NOT:
                ax = !ax;
                break;
            case OP_NEGATE:
                ax = -ax;
                break;
            case OP_STR:
                ax = ctx->heap + ax; // convert to string address
                break;
            case OP_PRINTF:
                printf("printf called with %d arguments\n", *pc);
                sp -= *pc; // pop arguments
                ax = printf(sp[0], sp[1], sp[2], sp[3], sp[4], sp[5]);
                pc++;
                break;
            default:
                fprintf(stderr, "Unknown opcode: %llu\n", ip);
                free(stk);
                return -1;
        }
    }
}
#include "runner.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "def.h"
#include "opcode.h"
#include "native.h"


void trace(context_t ctx){
    uint64_t* stk = NULL; // stack pointer
    uint64_t* bp = NULL;  // base pointer
    uint64_t* sp = NULL;  // stack pointer
    uint64_t* pc = ctx->btcode; // program counter
    uint64_t  ip = 0;     // instruction pointer
    type_t    ax = {};
    stk = bp = sp = (uint64_t*)malloc(8192 * sizeof(uint64_t)); // allocate stack memory
    if(!stk){
        fprintf(stderr, "Failed to allocate memory for stack\n");
        exit(EXIT_FAILURE);
    }
    const int print_header = __args__.debug && __args__.optimize != 1;
    for(ip = *pc; ip != 0; ip = *pc){
        pc++;
        if(print_header){
            printf("%3llu OP=%2llu, sp = %2lld, ax = %lld\n", pc - ctx->btcode - 1, ip, sp - stk, ax.uval);
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
                ax.uval = *(bp + *pc);
                pc++;
                break;
            case OP_S_LOC:
                *(bp + *pc) = ax.uval;
                pc++;
                break;
            case OP_IMM:
                ax.uval = *pc;
                pc++;
                break;
            case OP_ADJ:
                sp = bp + *pc;
                pc++;
                break;
            case OP_POP:
                sp--;
                ax.uval = *sp;
                break;
            case OP_FUNC: // 函数入口标记，什么都不做
                break;
            case OP_CALL:
                ax.uval = *(sp - *pc - 1);                               // 获取函数地址
                *(sp - *pc - 2) = (uint64_t)(bp - stk);             // 保存基指针位置
                *(sp - *pc - 1) = (uint64_t)(pc - ctx->btcode + 1); // 保存返回地址
                bp = sp - *pc; // 更新基指针
                if(ax.uval < 65535 && *(ctx->btcode + ax.uval) == OP_FUNC){
                    pc = ctx->btcode + ax.uval + 1;
                    break;
                }
                NativeFn fn = ax.pval;
                ax.uval = fn(bp, *pc);
            case OP_RET:
                pc = ctx->btcode + *(bp - 1); // 恢复返回地址
                sp = bp - 2; // 恢复栈指针
                bp = stk + *(bp - 2); // 恢复基指针
                break;
            case OP_EXIT:
                free(stk);
                FreeContext(ctx);
                exit((int)ax.ival);
                break;
            case OP_JMP:
                pc = ctx->btcode + *pc; // 跳转到指定地址
                break;
            case OP_JZ:
                if(ax.ival){
                    pc++;
                } else {
                    pc = ctx->btcode + *pc;
                }
                break;
            case OP_PUSH:
                *sp = ax.uval;
                sp++;
                break;
            case OP_G_OFF:{
                uint64_t offset = ax.uval;
                sp--;
                char *addr = (char*)(*sp);
                if(ctx->heap <= addr && addr < ctx->heap + ctx->heap_cur){
                    char chr = *(char*)(addr + offset);
                    ax.uval = (uint64_t)chr;
                } else {
                    ax.uval = *(uint64_t*)(addr + offset * sizeof(uint64_t));
                }
                break;
            }
            case OP_S_OFF:{
                sp--;
                uint64_t offset = *sp;
                sp--;
                char *addr = (char*)(*sp);
                if(ctx->heap <= addr && addr < ctx->heap + ctx->heap_cur){
                    *(char*)(addr + offset) = (char)ax.uval;
                } else {
                    *(uint64_t*)(addr + offset * sizeof(uint64_t)) = ax.uval;
                }
                break;
            }
            case OP_OR:
                sp--;
                ax.uval = (*sp | ax.uval);
                break;
            case OP_XOR:
                sp--;
                ax.uval = (*sp ^ ax.uval);
                break;
            case OP_AND:
                sp--;
                ax.uval = (*sp & ax.uval);
                break;
            case OP_EQU:
                sp--;
                ax.uval = (*sp == ax.uval);
                break;
            case OP_GRT:
                sp--;
                ax.uval = ((int64_t)*sp > (int64_t)ax.ival);
                break;
            case OP_LES:
                sp--;
                ax.uval = ((int64_t)*sp < (int64_t)ax.ival);
                break;
            case OP_SHL:
                sp--;
                ax.uval = (*sp << ax.uval);
                break;
            case OP_SHR:
                sp--;
                ax.uval = (*sp >> ax.uval);
                break;
            case OP_ADD:
                sp--;
                ax.uval = (*sp + ax.uval);
                break;
            case OP_SUB:
                sp--;
                ax.uval = (*sp - ax.uval);
                break;
            case OP_MUL:
                sp--;
                ax.uval = (*sp * ax.uval);
                break;
            case OP_DIV:
                if(ax.ival == 0){
                    fprintf(stderr, "Division by zero\n");
                    free(stk);
                    exit(EXIT_FAILURE);
                }
                sp--;
                ax.ival = (*sp / ax.ival);
                break;
            case OP_MOD:
                if(ax.ival == 0){
                    fprintf(stderr, "Division by zero\n");
                    free(stk);
                    exit(EXIT_FAILURE);
                }
                sp--;
                ax.ival = (*sp % ax.ival);
                break;
            case OP_NOT:
                ax.ival = !ax.ival;
                break;
            case OP_FLT:
                ax.fval = ax.ival;
                 break;
            case OP_INT:
                ax.ival = ax.fval;
                break;
            case OP_ADD_F:{
                sp--;
                type_t result;
                result.uval = *sp;
                result.fval += ax.fval;
                ax.uval = result.uval;
                break;
            }
            case OP_SUB_F:{
                sp--;
                type_t result;
                result.uval = *sp;
                result.fval -= ax.fval;
                ax.uval = result.uval;
                break;
            }
            case OP_MUL_F:{
                sp--;
                type_t result;
                result.uval = *sp;
                result.fval *= ax.fval;
                ax.uval = result.uval;
                break;
            }
            case OP_DIV_F:{
                sp--;
                if(ax.fval == 0.0){
                    fprintf(stderr, "Division by zero\n");
                    free(stk);
                    exit(EXIT_FAILURE);
                }
                type_t result;
                result.uval = *sp;
                result.fval = result.fval / ax.fval;
                ax.uval = result.uval;
                break;
            }
            case OP_EQU_F:
                sp--;
                ax.uval = ((type_t){.uval=*sp}.fval == ax.fval);
                break;
            case OP_GRT_F:
                sp--;
                ax.uval = ((type_t){.uval=*sp}.fval > ax.fval);
                break;
            case OP_LES_F:
                sp--;
                ax.uval = ((type_t){.uval=*sp}.fval < ax.fval);
                break;
            default:
                fprintf(stderr, "Unknown opcode: %llu in %d\n", ip, (int)(pc - ctx->btcode - 1));
                free(stk);
                exit(EXIT_FAILURE);
        }
    }
}

void eval(context_t ctx){
    uint64_t* volatile stk = NULL; // stack pointer
    uint64_t* volatile bp = NULL;  // base pointer
    uint64_t* volatile sp = NULL;  // stack pointer
    uint64_t* pc = ctx->btcode; // program counter
    uint64_t  ip = 0;     // instruction pointer
    type_t    ax = {};
    stk = bp = sp = (uint64_t*)malloc(8192 * sizeof(uint64_t)); // allocate stack memory
    if(!stk){
        fprintf(stderr, "Failed to allocate memory for stack\n");
        exit(EXIT_FAILURE);
    }
    static void* dispatch_table[] = {
        &&L_OP_ZERO, // 手动处理 OP_ZERO (值为0)
        #define OPCODE(name, desc) &&L_##name,
        #include "opcode.def"
        #undef OPCODE
    };
    #define DISPATCH() goto *dispatch_table[*pc++]
    DISPATCH();
    L_OP_G_GLO:
        ax = ctx->sym[*pc].val; // get global variable value
        pc++;
        DISPATCH();
    L_OP_S_GLO:
        ctx->sym[*pc].val = ax; // set global variable value
        pc++;
        DISPATCH();
    L_OP_G_LOC:
        ax.uval = *(bp + *pc);
        pc++;
        DISPATCH();
    L_OP_S_LOC:
        *(bp + *pc) = ax.uval;
        pc++;
        DISPATCH();
    L_OP_IMM:
        ax.uval = *pc;
        pc++;
        DISPATCH();
    L_OP_ADJ:
        sp = bp + *pc;
        pc++;
        DISPATCH();
    L_OP_POP:
        sp--;
        ax.uval = *sp;
        DISPATCH();
    L_OP_FUNC: // 函数入口标记，什么都不做
        DISPATCH();
    L_OP_CALL:
        ax.uval = *(sp - *pc - 1);                               // 获取函数地址
        *(sp - *pc - 2) = (uint64_t)(bp - stk);             // 保存基指针位置
        *(sp - *pc - 1) = (uint64_t)(pc - ctx->btcode + 1); // 保存返回地址
        bp = sp - *pc; // 更新基指针
        if(ax.uval < 65535 && *(ctx->btcode + ax.uval) == OP_FUNC){
            pc = ctx->btcode + ax.uval + 1;
            DISPATCH();
        }
        NativeFn fn = ax.pval;
        ax.uval = fn(bp, *pc);
    L_OP_RET:
        pc = ctx->btcode + *(bp - 1); // 恢复返回地址
        sp = bp - 2; // 恢复栈指针
        bp = stk + *(bp - 2); // 恢复基指针
        DISPATCH();
    L_OP_EXIT:
        free(stk);
        FreeContext(ctx);
        exit((int)ax.ival);
        DISPATCH();
    L_OP_JMP:
        pc = ctx->btcode + *pc; // 跳转到指定地址
        DISPATCH();
    L_OP_JZ:
        if(ax.ival){
            pc++;
        } else {
            pc = ctx->btcode + *pc;
        }
        DISPATCH();
    L_OP_PUSH:
        *sp = ax.uval;
        sp++;
        DISPATCH();
    L_OP_G_OFF:{
        uint64_t offset = ax.uval;
        sp--;
        char *addr = (char*)(*sp);
        if(ctx->heap <= addr && addr < ctx->heap + ctx->heap_cur){
            char chr = *(char*)(addr + offset);
            ax.uval = (uint64_t)chr;
        } else {
            ax.uval = *(uint64_t*)(addr + offset * sizeof(uint64_t));
        }
        DISPATCH();
    }
    L_OP_S_OFF:{
        sp--;
        uint64_t offset = *sp;
        sp--;
        char *addr = (char*)(*sp);
        if(ctx->heap <= addr && addr < ctx->heap + ctx->heap_cur){
            *(char*)(addr + offset) = (char)ax.uval;
        } else {
            *(uint64_t*)(addr + offset * sizeof(uint64_t)) = ax.uval;
        }
        DISPATCH();
    }
    L_OP_OR:
        sp--;
        ax.uval = (*sp | ax.uval);
        DISPATCH();
    L_OP_XOR:
        sp--;
        ax.uval = (*sp ^ ax.uval);
        DISPATCH();
    L_OP_AND:
        sp--;
        ax.uval = (*sp & ax.uval);
        DISPATCH();
    L_OP_EQU:
        sp--;
        ax.uval = (*sp == ax.uval);
        DISPATCH();
    L_OP_GRT:
        sp--;
        ax.uval = ((int64_t)*sp > (int64_t)ax.ival);
        DISPATCH();
    L_OP_LES:
        sp--;
        ax.uval = ((int64_t)*sp < (int64_t)ax.ival);
        DISPATCH();
    L_OP_SHL:
        sp--;
        ax.uval = (*sp << ax.uval);
        DISPATCH();
    L_OP_SHR:
        sp--;
        ax.uval = (*sp >> ax.uval);
        DISPATCH();
    L_OP_ADD:
        sp--;
        ax.uval = (*sp + ax.uval);
        DISPATCH();
    L_OP_SUB:
        sp--;
        ax.uval = (*sp - ax.uval);
        DISPATCH();
    L_OP_MUL:
        sp--;
        ax.uval = (*sp * ax.uval);
        DISPATCH();
    L_OP_DIV:
        if(ax.ival == 0){
            fprintf(stderr, "Division by zero\n");
            free(stk);
            exit(EXIT_FAILURE);
        }
        sp--;
        ax.ival = (*sp / ax.ival);
        DISPATCH();
    L_OP_MOD:
        if(ax.ival == 0){
            fprintf(stderr, "Division by zero\n");
            free(stk);
            exit(EXIT_FAILURE);
        }
        sp--;
        ax.ival = (*sp % ax.ival);
        DISPATCH();
    L_OP_NOT:
        ax.ival = !ax.ival;
        DISPATCH();
    L_OP_FLT:
        ax.fval = ax.ival;
            DISPATCH();
    L_OP_INT:
        ax.ival = ax.fval;
        DISPATCH();
    L_OP_ADD_F:{
        sp--;
        type_t result;
        result.uval = *sp;
        result.fval += ax.fval;
        ax.uval = result.uval;
        DISPATCH();
    }
    L_OP_SUB_F:{
        sp--;
        type_t result;
        result.uval = *sp;
        result.fval -= ax.fval;
        ax.uval = result.uval;
        DISPATCH();
    }
    L_OP_MUL_F:{
        sp--;
        type_t result;
        result.uval = *sp;
        result.fval *= ax.fval;
        ax.uval = result.uval;
        DISPATCH();
    }
    L_OP_DIV_F:{
        sp--;
        if(ax.fval == 0.0){
            fprintf(stderr, "Division by zero\n");
            free(stk);
            exit(EXIT_FAILURE);
        }
        type_t result;
        result.uval = *sp;
        result.fval = result.fval / ax.fval;
        ax.uval = result.uval;
        DISPATCH();
    }
    L_OP_EQU_F:
        sp--;
        ax.uval = ((type_t){.uval=*sp}.fval == ax.fval);
        DISPATCH();
    L_OP_GRT_F:
        sp--;
        ax.uval = ((type_t){.uval=*sp}.fval > ax.fval);
        DISPATCH();
    L_OP_LES_F:
        sp--;
        ax.uval = ((type_t){.uval=*sp}.fval < ax.fval);
        DISPATCH();
    L_OP_ZERO:
    L_OP_JEND:
        fprintf(stderr, "Unknown opcode: %llu in %d\n", ip, (int)(pc - ctx->btcode - 1));
        free(stk);
        exit(EXIT_FAILURE);
}

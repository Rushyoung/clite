#include "def.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "native.h"
#include "opcode.h"
#include "scanner.h"

#include "token.h"


context_t InitContext(){
    context_t ctx = malloc(sizeof(struct _context_t));
    ctx->loop_depth = 0;
    ctx->sym = malloc(8192 * sizeof(token_t));
    ctx->sym_idx = 0;
    ctx->sym_loc = NULL;

    ctx->heap = calloc(65536, 1);
    ctx->heap_cur = 0;

    ctx->btcode = calloc(65536 * sizeof(uint64_t), 1);
    ctx->btcode_cur = ctx->btcode;

    char* builtin = 
    "break char continue do else enum for if int return sizeof while void "
    "main open read close printf input malloc "
    "free memset memcmp exit time sleep rand "
    "EXIT_SUCCESS EXIT_FAILURE NULL EOF RAND_MAX";
    scanner keyword = InitScanner(193, builtin);

    for(int ids = TK_BREAK; ids <= TK_VOID; ids++){
        next(keyword, ctx);
        ctx->sym[ctx->sym_idx - 1].tk = ids;
    }

    next(keyword, ctx);
    ctx->sym[ctx->sym_idx - 1].tk = TK_ID;    // main function identifier
    ctx->sym[ctx->sym_idx - 1].val = 0;
    ctx->main_id = ctx->sym_idx - 1;          // store main function index

    NativeFn native_functions[] = {
        lite_open, lite_read, lite_close, lite_printf, lite_input,
        lite_malloc, lite_free, lite_memset, lite_memcmp, lite_exit,
        lite_time, lite_sleep, lite_rand
    };
    for(int ids = 0; ids < 13; ids++){
        next(keyword, ctx);
        ctx->sym[ctx->sym_idx - 1].class = TK_SYS; // system calls
        ctx->sym[ctx->sym_idx - 1].type  = TP_INT; // all system calls return int
        ctx->sym[ctx->sym_idx - 1].val   = (uint64_t)native_functions[ids];
    }
    
    uint64_t constants[] = {
        EXIT_SUCCESS, EXIT_FAILURE, NULL, EOF, RAND_MAX
    };
    for(int ids = 0; ids < 5; ids++){
        next(keyword, ctx);
        ctx->sym[ctx->sym_idx - 1].class = TK_SYS; // system constants
        ctx->sym[ctx->sym_idx - 1].type  = TP_INT; // all constants are int
        ctx->sym[ctx->sym_idx - 1].val   = constants[ids];
    }

    free(keyword);

    return ctx;
}


void FreeContext(context_t ctx){
    free(ctx->sym);
    free(ctx->heap);
    free(ctx->btcode);
    free(ctx);
}


void emit(context_t ctx, uint64_t op){
    if(ctx->btcode_cur - ctx->btcode >= 65536){
        perror("Bytecode buffer overflow");
        exit(EXIT_FAILURE);
    }
    *ctx->btcode_cur = op;
    ctx->btcode_cur++;
}

uint64_t* blank(context_t ctx){
    if(ctx->btcode_cur == ctx->btcode){
        perror("Bytecode buffer underflow");
        exit(EXIT_FAILURE);
    }
    uint64_t* addr = ctx->btcode_cur;
    ctx->btcode_cur++;
    return addr;
}

void patch(context_t ctx, uint64_t* addr, uint64_t op){
    if(addr < ctx->btcode || addr >= ctx->btcode_cur){
        perror("Patch address out of bounds");
        exit(EXIT_FAILURE);
    }
    *addr = op;
}


token_t* SymFind(context_t ctx, token_t tk){
    if(tk.tk != TK_ID){
        perror("SymFind() called with non-identifier token");
        exit(EXIT_FAILURE);
    }
    for(int i = ctx->sym_idx - 1; i >= 0; i--){
        if(ctx->sym[i].hash == tk.hash && 
           ctx->sym[i].len == tk.len &&
           strncmp(ctx->sym[i].name, tk.name, tk.len) == 0){
            return &ctx->sym[i];
        }
    }
    return NULL;
}


token_t* SymAdd(context_t ctx, token_t tk){
    if(tk.tk != TK_ID){
        perror("SymAdd() called with non-identifier token");
        exit(EXIT_FAILURE);
    }
    if(ctx->sym_idx >= 8192){
        perror("Symbol table overflow");
        exit(EXIT_FAILURE);
    }
    ctx->sym[ctx->sym_idx] = tk;
    ctx->sym[ctx->sym_idx].class = 0; // default class is 0
    ctx->sym[ctx->sym_idx].val = 0;   // default value is 0
    ctx->sym_idx++;
    return &ctx->sym[ctx->sym_idx - 1];
}


void SymSetloc(context_t ctx){
    if(ctx->sym_loc != NULL){
        printf("SymSetloc() called with an existing local symbol table");
        exit(EXIT_FAILURE);
    }
    ctx->sym_loc = ctx->sym + ctx->sym_idx;
}


void SymEndloc(context_t ctx){
    if(ctx->sym_loc == NULL){
        printf("SymEndloc() called without a local symbol table");
        exit(EXIT_FAILURE);
    }
    ctx->sym_idx = ctx->sym_loc - ctx->sym;
    ctx->sym_loc = NULL; // reset local symbol table pointer
}


void SymStartLoop(context_t ctx){
    ctx->loop_depth++;
}


void SymEndLoop(context_t ctx){
    ctx->loop_depth--;
}


int SymLoopDepth(context_t ctx){
    return ctx->loop_depth;
}


struct _args_t __args__;

void InitArgs(int argc, char* argv[]){
    __args__.debug = 0;
    __args__.bytecode = 0;
    __args__.inputs = NULL;
    __args__.symboltable = 0; // 初始化 symboltable 标志

    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-d") == 0){
            __args__.debug = 1;
        } else if(strcmp(argv[i], "--bytecode") == 0 || strcmp(argv[i], "-b") == 0){
            __args__.bytecode = 1;
        } else if(strcmp(argv[i], "--symboltable") == 0 || strcmp(argv[i], "-s") == 0){ // 添加 symboltable 选项
            __args__.symboltable = 1;
        } else if(__args__.inputs == NULL){
            __args__.inputs = argv[i];
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            exit(EXIT_FAILURE);
        }
    }
}

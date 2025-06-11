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
    ctx->sym = malloc(8192 * sizeof(token_t));
    ctx->sym_idx = 0;
    ctx->sym_loc = NULL;

    ctx->heap = malloc(65536);
    memset(ctx->heap, 0, 65536);
    ctx->heap_cur = 0;

    ctx->btcode = malloc(65536 * sizeof(uint64_t));
    memset(ctx->btcode, 0, 65536 * sizeof(uint64_t));
    ctx->btcode_cur = ctx->btcode;

    char* builtin = 
    "char else enum for if int return sizeof while void main "
    "open read close printf malloc free memset memcmp exit";
    scanner keyword = InitScanner(106, builtin);

    for(int ids = TK_CHAR; ids <= TK_VOID; ids++){
        next(keyword, ctx);
        ctx->sym[ctx->sym_idx - 1].tk = ids;
    }

    next(keyword, ctx);
    ctx->sym[ctx->sym_idx - 1].tk = TK_ID;    // main function identifier
    ctx->sym[ctx->sym_idx - 1].val = 0;
    ctx->main_id = ctx->sym_idx - 1;          // store main function index

    NativeFn native_functions[] = {
        lite_open, lite_read, lite_close, lite_printf,
        lite_malloc, lite_free, lite_memset, lite_memcmp, lite_exit
    };
    for(int ids = OP_OPEN; ids <= OP_EXIT; ids++){
        next(keyword, ctx);
        ctx->sym[ctx->sym_idx - 1].class = TK_SYS; // system calls
        ctx->sym[ctx->sym_idx - 1].type  = TP_INT; // all system calls return int
        ctx->sym[ctx->sym_idx - 1].val   = (uint64_t)native_functions[ids - OP_OPEN];
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

uint64_t* black(context_t ctx){
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

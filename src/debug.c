#include "debug.h"

#include <stdio.h>

#include "def.h"
#include "opcode.h"

char* token_name[256] = {
    [TK_NUM] = "TK_NUM",
    [TK_FUN] = "TK_FUN",
    [TK_SYS] = "TK_SYS",
    [TK_GLO] = "TK_GLO",
    [TK_LOC] = "TK_LOC",
    [TK_ID] = "TK_ID",
    [TK_STR] = "TK_STR",
    [TK_CHAR] = "TK_CHAR",
    [TK_ELSE] = "TK_ELSE",
    [TK_ENUM] = "TK_ENUM",
    [TK_IF] = "TK_IF",
    [TK_INT] = "TK_INT",
    [TK_RETURN] = "TK_RETURN",
    [TK_SIZEOF] = "TK_SIZEOF",
    [TK_WHILE] = "TK_WHILE",
    [TK_VOID] = "TK_VOID",
    [TK_ASSIGN] = "TK_ASSIGN",
    [TK_COND] = "TK_COND",
    [TK_LOR] = "TK_LOR",
    [TK_LAN] = "TK_LAN",
    [TK_NOT] = "TK_NOT",
    [TK_OR] = "TK_OR",
    [TK_XOR] = "TK_XOR",
    [TK_AND] = "TK_AND",
    [TK_EQ] = "TK_EQ",
    [TK_NE] = "TK_NE",
    [TK_LT] = "TK_LT",
    [TK_GT] = "TK_GT",
    [TK_LE] = "TK_LE",
    [TK_GE] = "TK_GE",
    [TK_SHL] = "TK_SHL",
    [TK_SHR] = "TK_SHR",
    [TK_ADD] = "TK_ADD",
    [TK_SUB] = "TK_SUB",
    [TK_MUL] = "TK_MUL",
    [TK_DIV] = "TK_DIV",
    [TK_MOD] = "TK_MOD",
    [TK_INC] = "TK_INC",
    [TK_DEC] = "TK_DEC",
    [TK_LEFT_PAREN] = "TK_LEFT_PAREN",
    [TK_RIGHT_PAREN] = "TK_RIGHT_PAREN",
    [TK_LEFT_BRACE] = "TK_LEFT_BRACE",
    [TK_RIGHT_BRACE] = "TK_RIGHT_BRACE",
    [TK_LEFT_BRACKET] = "TK_LEFT_BRACKET",
    [TK_RIGHT_BRACKET] = "TK_RIGHT_BRACKET",
    [TK_COMMA] = "TK_COMMA",
    [TK_SEMICOLON] = "TK_SEMICOLON",
    [TK_COLON] = "TK_COLON",
};
char* op_name[64] = {
    [OP_G_GLO] = "OP_G_GLO",
    [OP_S_GLO] = "OP_S_GLO",
    [OP_G_LOC] = "OP_G_LOC",
    [OP_S_LOC] = "OP_S_LOC",
    [OP_IMM] = "OP_IMM",
    [OP_JMP] = "OP_JMP",
    [OP_JZ] = "OP_JZ",
    [OP_CALL] = "OP_CALL",
    [OP_RET] = "OP_RET",
    [OP_SAD] = "OP_SAD",
    [OP_PUSH] = "OP_PUSH",
    [OP_OR] = "OP_OR",
    [OP_XOR] = "OP_XOR",
    [OP_AND] = "OP_AND",
    [OP_EQU] = "OP_EQU",
    [OP_LES] = "OP_LES",
    [OP_GRT] = "OP_GRT",
    [OP_SHL] = "OP_SHL",
    [OP_SHR] = "OP_SHR",
    [OP_ADD] = "OP_ADD",
    [OP_SUB] = "OP_SUB",
    [OP_MUL] = "OP_MUL",
    [OP_DIV] = "OP_DIV",
    [OP_MOD] = "OP_MOD",
    [OP_NOT] = "OP_NOT",
    [OP_NEGATE] = "OP_NEGATE",
    [OP_STR] = "OP_STR",
    [OP_OPEN] = "OPEN_FILE", 
    [OP_READ] = "READ_FILE", 
    [OP_CLOSE] = "CLOSE_FILE", 
    [OP_PRINTF] = "PRINTF", 
    [OP_MALLOC] = "MALLOC", 
    [OP_FREE] = "FREE", 
    [OP_MEMSET] = "MEMSET", 
    [OP_MEMCMP] = "MEMCMP", 
    [OP_EXIT] = "EXIT"
};


void DumpToken(context_t ctx, token_t tk) {
    printf("Token: %03d ", tk.tk);
    if(tk.tk < 256 && tk.tk >= 128) {
        printf("%9s | ", token_name[tk.tk]);
    } else {
        printf("%9c | ", (char)tk.tk);
    }
    switch(tk.tk) {
        case TK_NUM:
            printf("value: %lld", tk.val);
            break;
        case TK_ID:
            printf("len: %d, value: %.*s", tk.len, tk.len, tk.name);
            break;
        case TK_STR:
            printf("len: %d, value: \"%.*s\"", tk.len, tk.len, ctx->heap + tk.val);
            break;
    }
    printf("\n");
}


void DumpScanner(scanner sc) {
    context_t ctx = InitContext();
    if (sc == NULL) {
        printf("Scanner is NULL\n");
        return;
    }
    
    sc->cur = sc->src;
    sc->pre = NULL;
    next(sc, ctx); // 初始化扫描器
    for(token_t tk = prst(sc, ctx); tk.tk; tk = prst(sc, ctx)) {
        DumpToken(ctx, tk);
        next(sc, ctx);
    }
    FreeContext(ctx);
}


void DumpBtcode(context_t ctx){
    if (ctx == NULL || ctx->btcode == NULL) {
        printf("Bytecode is NULL\n");
        return;
    }
    uint64_t* cur = ctx->btcode;
    while (cur < ctx->btcode_cur) {
        uint64_t op = *cur++;
        printf("%3u: %02d ", cur - ctx->btcode - 1, op);
        if (op < 64) {
            printf("%-10s ", op_name[op]);
        } else {
            printf("%-10s ", "UNKNOWN");
        }
        if((OP_G_GLO <= op && op <= OP_CALL) || // 1-8 are variable operations
           (OP_OPEN <= op && op <= OP_EXIT)) { // 24-32 are system calls
            printf("value: %lld\n", *cur++);
        } else if (op == OP_JMP || op == OP_JZ) {
            printf("Jump to: %lld\n", *cur++);
        } else {
            printf("\n");
        }
    }
}
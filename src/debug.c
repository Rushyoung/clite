#include "debug.h"

#include <stdio.h>

#include "def.h"
#include "opcode.h"
#include "token.h"

#define TOKEN(name, desc) [name] = #name,
char* token_name[256] = {
    #include "token.def"
};
#undef TOKEN

#define OPCODE(name, desc) [name] = #name,
char* op_name[64] = {
    #include "opcode.def"
};
#undef OPCODE


void DumpToken(context_t ctx, token_t tk) {
    printf("Token(%03d): ", tk.tk);
    if(tk.tk < 256 && tk.tk >= 128) {
        printf("%12s | ", token_name[tk.tk]);
    } else {
        printf("%12c | ", (char)tk.tk);
    }
    switch(tk.tk) {
        case TK_NUM:
            if(tk.type == TYPE_INT){
                printf("int value: %llu", tk.val.uval);
            } else if(tk.type == TYPE_FLOAT) {
                printf("flt value: %lf", tk.val.fval);
            }
            break;
        case TK_ID:
            printf("len: %d, value: %.*s", tk.len, tk.len, tk.name);
            break;
        case TK_STR:
            printf("len: %d, value: \"%.*s\"", tk.len, tk.len, ctx->heap + tk.val.uval);
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
    for(token_t tk = prst(sc); tk.tk; tk = prst(sc)) {
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
        if (op == OP_JMP || op == OP_JZ) {
            printf("Jump to: %lld\n", *cur++);
        } else if(OP_G_GLO <= op && op <= OP_CALL){ // 1-8 are variable operations
            printf("value: %lld\n", *cur++);
        } else {
            printf("\n");
        }
    }
}

void DumpSymbol(context_t ctx) { // Renamed from DumpSymtable to match previous suggestion
    if (ctx == NULL || ctx->sym == NULL) {
        printf("Symbol table is NULL.\n");
        return;
    }
    printf("\n--- Symbol Table Dump (Size: %zu) ---\n", ctx->sym_idx);
    printf("Idx | TkType    | Name (Len)           | Class     | CType        | Hash       | Val (Offset/Addr)\n");

    for (size_t i = 0; i < ctx->sym_idx; ++i) {
        token_t s = ctx->sym[i];
        char name_display_buffer[25]; // Buffer for displaying name and length

        printf("%3zu | ", i);

        // 打印 TkType
        printf("%-9s | ", (s.tk >= TK_NUM && s.tk < 256 && token_name[s.tk]) ? token_name[s.tk] : "OTHER_TK");

        // 打印 Name 和 Len
        if (s.name != NULL && s.len > 0) {
            snprintf(name_display_buffer, sizeof(name_display_buffer), "%.*s (%zu)", (int)s.len, s.name, s.len);
        } else {
            snprintf(name_display_buffer, sizeof(name_display_buffer), "-");
        }
        printf("%-20s | ", name_display_buffer);

        // 打印 Class
        printf("%-9s | ", (s.class >= TK_NUM && s.class < 256 && token_name[s.class]) ? token_name[s.class] : (s.class == 0 ? "NO_CLASS" : "OTHER_CLS"));

        // 打印 CType (type 字段) - 直接在此处处理
        int ctype_val = s.type;
        int base_type = ctype_val;
        int ptr_level = 0;
        char ctype_str_buffer[32]; // 局部缓冲区用于构建类型字符串
        char* ctype_ptr = ctype_str_buffer;
        int remaining_space = sizeof(ctype_str_buffer);

        while (base_type >= TYPE_PTR) {
            base_type -= TYPE_PTR;
            ptr_level++;
        }

        const char* base_type_name;
        switch (base_type) {
            case TYPE_VOID: base_type_name = "void"; break;
            case TYPE_CHAR: base_type_name = "char"; break;
            case TYPE_INT:  base_type_name = "int";  break;
            default:      base_type_name = "unk_base"; break;
        }

        int written = snprintf(ctype_ptr, remaining_space, "%s", base_type_name);
        if (written > 0 && written < remaining_space) {
            ctype_ptr += written;
            remaining_space -= written;
        } else {
            // 如果基础类型名称写入失败或填满了缓冲区，则截断
            ctype_str_buffer[sizeof(ctype_str_buffer)-1] = '\0';
        }

        for (int k = 0; k < ptr_level && remaining_space > 1; ++k) {
            *ctype_ptr++ = '*';
            remaining_space--;
        }
        *ctype_ptr = '\0'; // 确保空终止

        printf("%-12s | ", ctype_str_buffer);

        // 打印 Hash
        printf("0x%08X | ", s.hash);

        // 打印 Val
        printf("%llu\n", s.val);
    }
    printf("--- End of Symbol Table Dump ---\n");
}
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

#include "def.h"
#include "opcode.h"
#include "scanner.h"

#include "token.h"
#include "debug.h"

#define log(s) printf("At %s:%d\n%s: ", __FUNCTION__, __LINE__, #s);s

ParseRule Rules[] = {
    [TK_NUM] = {NULL,            NULL,           PREC_NONE },
    [TK_FUN] = {NULL,            NULL,           PREC_NONE },
    [TK_SYS] = {NULL,            NULL,           PREC_NONE },
    [TK_GLO] = {NULL,            NULL,           PREC_NONE },
    [TK_LOC] = {NULL,            NULL,           PREC_NONE },
    [TK_ID] = {NULL,            NULL,           PREC_NONE },
    [TK_STR] = {NULL,            NULL,           PREC_NONE },
    [TK_CHAR] = {NULL,            NULL,           PREC_NONE },
    [TK_ELSE] = {NULL,            NULL,           PREC_NONE },
    [TK_ENUM] = {NULL,            NULL,           PREC_NONE },
    [TK_IF] = {NULL,            NULL,           PREC_NONE },
    [TK_INT] = {NULL,            NULL,           PREC_NONE },
    [TK_RETURN] = {NULL,            NULL,           PREC_NONE },
    [TK_SIZEOF] = {NULL,            NULL,           PREC_NONE },
    [TK_WHILE] = {NULL,            NULL,           PREC_NONE },
    [TK_VOID] = {NULL,            NULL,           PREC_NONE },
    [TK_ASSIGN] = {NULL,            NULL,           PREC_NONE },
    [TK_COND] = {NULL,            NULL,           PREC_NONE },
    [TK_LOR] = {NULL,            NULL,           PREC_NONE },
    [TK_LAN] = {NULL,            NULL,           PREC_NONE },
    [TK_NOT] = {NULL,            NULL,           PREC_NONE },
    [TK_OR] = {NULL,            NULL,           PREC_NONE },
    [TK_XOR] = {NULL,            NULL,           PREC_NONE },
    [TK_AND] = {NULL,            NULL,           PREC_NONE },
    [TK_EQ] = {NULL,            NULL,           PREC_NONE },
    [TK_NE] = {NULL,            NULL,           PREC_NONE },
    [TK_LT] = {NULL,            NULL,           PREC_NONE },
    [TK_GT] = {NULL,            NULL,           PREC_NONE },
    [TK_LE] = {NULL,            NULL,           PREC_NONE },
    [TK_GE] = {NULL,            NULL,           PREC_NONE },
    [TK_SHL] = {NULL,            NULL,           PREC_NONE },
    [TK_SHR] = {NULL,            NULL,           PREC_NONE },
    [TK_ADD] = {NULL,            NULL,           PREC_NONE },
    [TK_SUB] = {NULL,            NULL,           PREC_NONE },
    [TK_MUL] = {NULL,            NULL,           PREC_NONE },
    [TK_DIV] = {NULL,            NULL,           PREC_NONE },
    [TK_MOD] = {NULL,            NULL,           PREC_NONE },
    [TK_INC] = {NULL,            NULL,           PREC_NONE },
    [TK_DEC] = {NULL,            NULL,           PREC_NONE },
    [TK_LEFT_PAREN] = {NULL,            NULL,           PREC_NONE },
    [TK_RIGHT_PAREN] = {NULL,            NULL,           PREC_NONE },
    [TK_LEFT_BRACE] = {NULL,            NULL,           PREC_NONE },
    [TK_RIGHT_BRACE] = {NULL,            NULL,           PREC_NONE },
    [TK_LEFT_BRACKET] = {NULL,            NULL,           PREC_NONE },
    [TK_RIGHT_BRACKET] = {NULL,            NULL,           PREC_NONE },
    [TK_COMMA] = {NULL,            NULL,           PREC_NONE },
    [TK_SEMICOLON] = {NULL,            NULL,           PREC_NONE },
    [TK_COLON] = {NULL,            NULL,           PREC_NONE },
};

static int match(context_t ctx, scanner sc, TkType tk) {
    token_t current = prst(sc, ctx);
    if(current.tk == tk) {
        next(sc, ctx);
        return 1; // 匹配成功
    }
    return 0; // 匹配失败
}

static void consume(context_t ctx, scanner sc, TkType tk, char* msg) {
    token_t current = prst(sc, ctx);
    if(current.tk != tk) {
        printf(msg);
        exit(EXIT_FAILURE);
    }
    next(sc, ctx); // 跳过匹配的 token
}


static int __ctype(context_t ctx, scanner sc) {
    token_t tk = prst(sc, ctx);
    int type = TP_INT; // 默认类型为整型
    switch(tk.tk) {
        case TK_INT:
            type = TP_INT;
            break;
        case TK_CHAR:
            type = TP_CHAR;
            break;
        case TK_VOID:
            type = TP_VOID;
            break;
        default:
            printf("Expected type declaration (int, char, void)");
            exit(EXIT_FAILURE);
    }
    next(sc, ctx); // 跳过类型声明
    return type; // 返回解析的类型
}

static token_t* __identifier(context_t ctx, scanner sc, int* type) {
    while(match(ctx, sc, TK_MUL)) {
        *type += TP_PTR; // 处理指针类型
    }
    consume(ctx, sc, TK_ID, "Expected identifier after type declaration");
    return SymFind(ctx, prev(sc, ctx));
}


void parse_expr(context_t ctx, scanner sc, PrecLv level) {
    next(sc, ctx); // 跳过当前 token
    ParseFn prefixFn = Rules[prev(sc, ctx).tk].prefix; // 获取解析函数
    if(prefixFn == NULL) {
        printf("Expected expression, but something else found");
        exit(EXIT_FAILURE);
    }
    int can_assign = level <= PREC_ASSIGNMENT; // 是否允许赋值
    prefixFn(PassFunctionArgs); // 调用前缀解析函数
    while(level < Rules[prst(sc, ctx).tk].prec) {
        next(sc, ctx);
        ParseFn infixFn = Rules[prev(sc, ctx).tk].infix; // 获取中缀解析函数
        if(infixFn == NULL) {
            return;
        }
        next(sc, ctx); // 跳过当前 token
        infixFn(PassFunctionArgs); // 调用中缀解析函数
    }
}


void parse_stmt(context_t ctx, scanner sc) {
    /*token_t tk = prst(sc, ctx);
    log(DumpToken(ctx, tk));
    switch(tk.tk) {
        case TK_IF:
            stmt_if(ctx, sc);
            break;
        case TK_WHILE:
            stmt_while(ctx, sc);
            break;
        case TK_RETURN:
            stmt_return(ctx, sc);
            break;
        case '{':
            stmt_block(ctx, sc);
            break;
        case TK_INT:
        case TK_CHAR:
        case TK_VOID:
            stmt_decl(ctx, sc);
            break;
        default:
            stmt_expr(ctx, sc);
            log(DumpToken(ctx, prst(ctx, sc)));
            break;
    }*/
}

void parse_global(context_t ctx, scanner sc) {
    int base_type = TP_INT; // 基本类型，默认为整型
    int real_type = TP_INT; // 实际类型，默认为整型
    real_type = base_type = __ctype(ctx, sc); // 解析类型声明
    token_t* id = __identifier(ctx, sc, &real_type); // 解析标识符
    if(id->class == TK_GLO || id->class == TK_FUN) {
        printf("Global variable '%.*s' already defined", id->name, id->len);
        exit(EXIT_FAILURE);
    }
    id->type = real_type; // 设置变量类型
    if(match(ctx, sc, '(')){    // 函数声明
        id->class = TK_FUN; // 设置为函数
        id->val = ctx->btcode_cur - ctx->btcode; // 函数地址为当前字节码位置
        SymSetloc(ctx);   // 开始新的符号表作用域
        int arg_count = 0; // 函数参数计数
        while(prst(sc, ctx).tk != ')') {
            if(arg_count > 0) {
                consume(ctx, sc, ',', "Expected ',' in function argument list");
            }
            int arg_type = __ctype(ctx, sc); // 解析参数类型
            token_t* arg_id = __identifier(ctx, sc, &arg_type); // 解析参数标识符
            if(arg_id->class == TK_LOC) {
                printf("Function argument '%.*s' already defined", arg_id->name, arg_id->len);
                exit(EXIT_FAILURE);
            } else if(arg_id->class == TK_GLO || arg_id->class == TK_FUN) {
                arg_id = SymAdd(ctx, *arg_id); // 如果是全局变量或函数，则添加到符号表
            }
            arg_id->type = arg_type; // 设置参数类型
            arg_id->class = TK_LOC; // 设置为局部变量
            arg_id->val = arg_id - ctx->sym_loc; // 计算局部变量的偏移量
            arg_count++;
        }
        consume(ctx, sc, ')', "Expected ')' after function arguments");
        consume(ctx, sc, '{', "Expected '{' after function declaration");
        //stmt_block(ctx, sc); // 解析函数体
        consume(ctx, sc, '}', "Expected '}' after function body");
        SymEndloc(ctx); // 结束符号表作用域
    } else {      
        id->class = TK_GLO; // 设置为全局变量
        id->val = ctx->btcode_cur - ctx->btcode; // 全局变量地址为当前字节码位置
        emit(ctx, OP_S_GLO); // 生成全局变量存储指令
        emit(ctx, id->val);   // 存储全局变量的偏移量
        if(match(ctx, sc, TK_ASSIGN)) { // 如果有初始化赋值
            //parse_expr(ctx, sc, TK_ASSIGN); // 解析赋值表达式
        } else {
            emit(ctx, OP_IMM); // 初始化为0
            emit(ctx, 0);
        }
        emit(ctx, OP_S_GLO); // 存储全局变量值
        emit(ctx, id->val);   // 使用全局变量的偏移量
        consume(ctx, sc, ';', "Expected ';' after global variable declaration"); // 暂时不处理多个变量声明
    }
}


void compile(context_t ctx, scanner sc) {
    next(sc, ctx); // 开始解析，扫描第一个token
    while(!match(ctx, sc, 0)) {
        parse_global(ctx, sc);
    }
}
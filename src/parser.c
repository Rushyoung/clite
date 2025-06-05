#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

#include "def.h"
#include "opcode.h"
#include "scanner.h"

#include "token.h"
#include "debug.h"

#define log(s) printf("At %s:%d\n%s: ", __FUNCTION__, __LINE__, #s);s

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


int parse_expr(context_t ctx, scanner sc, int level) {
    next(sc, ctx); 
    token_t tk = prev(sc, ctx);
    log(DumpToken(ctx, tk));
    int expr_type = TP_INT;
    switch(tk.tk){
        case TK_NUM:
            emit(ctx, OP_IMM);
            emit(ctx, tk.val);
            break;
        case TK_STR:
            emit(ctx, OP_IMM);
            emit(ctx, tk.val);
            emit(ctx, OP_STR);
            break;
        case TK_ID: {
            token_t* id = SymFind(ctx, tk);
            tk = next(sc, ctx);
            if(tk.tk == '(') {
                tk = next(sc, ctx);
                int arg_count = 0;
                while(tk.tk != ')') {
                    parse_expr(ctx, sc, TK_ASSIGN);
                    emit(ctx, OP_PUSH);
                    arg_count++;
                    tk = prst(sc, ctx);
                    if(tk.tk == ',') {
                        tk = next(sc, ctx); // 跳过逗号
                    } else if(tk.tk != ')') {
                        printf("Expected ',' or ')' in function argument list");
                        exit(EXIT_FAILURE);
                    }
                }
                if(id->class == TK_FUN) {
                    emit(ctx, OP_SAD);  // 保存地址
                    emit(ctx, OP_IMM);
                    emit(ctx, id->val);
                    emit(ctx, OP_CALL);
                    emit(ctx, arg_count);
                } else if(id->class == TK_SYS){
                    emit(ctx, id->val); // 系统调用
                    emit(ctx, arg_count);
                } else {
                    printf("Function call to non-function identifier");
                    exit(EXIT_FAILURE);
                }
                next(sc, ctx); // 跳过 ')'
                log(DumpToken(ctx, prst(sc, ctx)));
            } else {
                int opcode;
                if(id->class == TK_GLO) {
                    opcode = OP_G_GLO;
                } else if(id->class == TK_LOC) {
                    opcode = OP_G_LOC;
                } else {
                    printf("Identifier is not variable");
                    exit(EXIT_FAILURE);
                }
                emit(ctx, opcode);
                emit(ctx, id->val);           
            }
            break;
        }
        default:
            printf("Unexpected token in expression");
            exit(EXIT_FAILURE);
    }

    while(tk.tk >= level){
        switch(tk.tk) {
            case TK_ADD:
                emit(ctx, OP_PUSH);
                next(sc, ctx);
                parse_expr(ctx, sc, TK_MUL);
                emit(ctx, OP_ADD);
                break;
            case TK_SUB:
                emit(ctx, OP_PUSH);
                next(sc, ctx);
                parse_expr(ctx, sc, TK_MUL);
                emit(ctx, OP_SUB);
                break;
            case TK_MUL:
                emit(ctx, OP_PUSH);
                next(sc, ctx);
                parse_expr(ctx, sc, TK_INC);
                emit(ctx, OP_MUL);
                break;
            case TK_DIV:
                emit(ctx, OP_PUSH);
                next(sc, ctx);
                parse_expr(ctx, sc, TK_INC);
                emit(ctx, OP_DIV);
                break;
            case TK_MOD:
                emit(ctx, OP_PUSH);
                next(sc, ctx);
                parse_expr(ctx, sc, TK_INC);
                emit(ctx, OP_MOD);
                break;
        }
        tk = prst(sc, ctx);
    }
    return expr_type;
}


static void stmt_if(context_t ctx, scanner sc) {
}

static void stmt_while(context_t ctx, scanner sc) {
}

static void stmt_return(context_t ctx, scanner sc) {
    token_t tk = next(sc, ctx);
    if(tk.tk != ';') {
        parse_expr(ctx, sc, TK_ASSIGN);
    }
    if(prst(sc, ctx).tk != ';') {
        printf("Expected ';' after return statement");
        exit(EXIT_FAILURE);
    }
    emit(ctx, OP_RET);
}

static void stmt_block(context_t ctx, scanner sc) {
    token_t tk = next(sc, ctx);
    while(tk.tk != '}') {
        parse_stmt(ctx, sc);
        tk = next(sc, ctx);
    }
}

static void stmt_decl(context_t ctx, scanner sc) {
    token_t tk = prst(sc, ctx);
    int base_type = TP_INT;
    int real_type = TP_INT;
    switch(tk.tk){
        case TK_CHAR:
            base_type = TP_CHAR;
        case TK_VOID:
            base_type = TK_VOID;
    }
    while(tk.tk != ';'){
        tk = next(sc, ctx);
        while(tk.tk == TK_MUL) {
            real_type += TP_PTR; // 处理指针类型
            tk = next(sc, ctx);
        }
        if(tk.tk != TK_ID) {
            printf("Expected identifier after type declaration");
            exit(EXIT_FAILURE);
        }/* todo: 这里需要检查是否已经定义过该标识符, 但是暂时允许重复定义
        if(tk.val ) {
            printf("Multiple definitions of identifier");
            exit(EXIT_FAILURE);
        }*/
        token_t* id = SymFind(ctx, tk);
        id->type = real_type;
        id->class = TK_LOC;
        id->val = id - ctx->sym_loc; // 计算局部变量的偏移量
        tk = next(sc, ctx);
        if(tk.tk == TK_ASSIGN) {
            next(sc, ctx); // 跳过 '='
            parse_expr(ctx, sc, TK_ASSIGN);
        } else {
            emit(ctx, OP_IMM); // 初始化为0
            emit(ctx, 0);
        }
        emit(ctx, OP_S_LOC);  // 设置变量值
        emit(ctx, id->val);   // 使用局部变量的偏移量
        if(prst(sc, ctx).tk == ','){
            continue; // 继续下一个变量声明
        }
        if(prst(sc, ctx).tk != ';') {
            printf("Expected ',' or ';' after variable declaration");
            exit(EXIT_FAILURE);
        } else {
            return; // 结束当前声明
        }
    }
}

static void stmt_expr(context_t ctx, scanner sc) {
    parse_expr(ctx, sc, TK_ASSIGN);
    token_t tk = prst(sc, ctx);
    if(tk.tk != ';') {
        printf("Expected ';' after expression statement");
        exit(EXIT_FAILURE);
    }
}

void parse_stmt(context_t ctx, scanner sc) {
    token_t tk = prst(sc, ctx);
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
    }
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
            if()
            arg_id->type = arg_type; // 设置参数类型
            arg_id->class = TK_LOC; // 设置为局部变量
            arg_id->val = arg_id - ctx->sym_loc; // 计算局部变量的偏移量
            arg_count++;
        }
        consume(ctx, sc, ')', "Expected ')' after function arguments");
        consume(ctx, sc, '{', "Expected '{' after function declaration");
        stmt_block(ctx, sc); // 解析函数体
        consume(ctx, sc, '}', "Expected '}' after function body");
        SymEndloc(ctx); // 结束符号表作用域
    } else {      
        id->class = TK_GLO; // 设置为全局变量
        id->val = ctx->btcode_cur - ctx->btcode; // 全局变量地址为当前字节码位置
        emit(ctx, OP_S_GLO); // 生成全局变量存储指令
        emit(ctx, id->val);   // 存储全局变量的偏移量
        if(match(ctx, sc, TK_ASSIGN)) { // 如果有初始化赋值
            parse_expr(ctx, sc, TK_ASSIGN); // 解析赋值表达式
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
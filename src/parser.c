#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

#include "def.h"
#include "opcode.h"
#include "scanner.h"

#include "token.h"
#include "debug.h"

#define log(s) printf("At %s:%d\n%s: ", __FUNCTION__, __LINE__, #s);s

// 定义解析函数
static void expr_number(ParseFunctionArgs);
static void expr_unary(ParseFunctionArgs);
static void expr_binary(ParseFunctionArgs);
static void expr_variable(ParseFunctionArgs);
static void expr_string(ParseFunctionArgs);

static void stmt_expr(ParseFunctionArgs);
static void stmt_block(ParseFunctionArgs);

ParseRule Rules[] = {//infix,          prefix,         precedence
    [TK_NUM]       = {expr_number,     NULL,           PREC_NONE },
    [TK_FUN]       = {NULL,            NULL,           PREC_NONE },
    [TK_SYS]       = {NULL,            NULL,           PREC_NONE },
    [TK_GLO]       = {NULL,            NULL,           PREC_NONE },
    [TK_LOC]       = {NULL,            NULL,           PREC_NONE },
    [TK_ID]        = {expr_variable,   NULL,           PREC_NONE },
    [TK_STR]       = {expr_string,     NULL,           PREC_NONE },
    [TK_CHAR]      = {NULL,            NULL,           PREC_NONE },
    [TK_ELSE]      = {NULL,            NULL,           PREC_NONE },
    [TK_ENUM]      = {NULL,            NULL,           PREC_NONE },
    [TK_IF]        = {NULL,            NULL,           PREC_NONE },
    [TK_INT]       = {NULL,            NULL,           PREC_NONE },
    [TK_RETURN]    = {NULL,            NULL,           PREC_NONE },
    [TK_SIZEOF]    = {NULL,            NULL,           PREC_NONE },
    [TK_WHILE]     = {NULL,            NULL,           PREC_NONE },
    [TK_VOID]      = {NULL,            NULL,           PREC_NONE },
    [TK_ASSIGN]    = {NULL,            NULL,           PREC_NONE },
    [TK_COND]      = {NULL,            NULL,           PREC_NONE },
    [TK_LOR]       = {NULL,            NULL,           PREC_NONE },
    [TK_LAN]       = {NULL,            NULL,           PREC_NONE },
    [TK_NOT]       = {expr_unary,      NULL,           PREC_NONE },
    [TK_OR]        = {NULL,            expr_binary,    PREC_BITWISE },
    [TK_XOR]       = {NULL,            expr_binary,    PREC_BITWISE },
    [TK_AND]       = {NULL,            expr_binary,    PREC_BITWISE },
    [TK_EQ]        = {NULL,            NULL,           PREC_NONE },
    [TK_NE]        = {NULL,            NULL,           PREC_NONE },
    [TK_LT]        = {NULL,            NULL,           PREC_NONE },
    [TK_GT]        = {NULL,            NULL,           PREC_NONE },
    [TK_LE]        = {NULL,            NULL,           PREC_NONE },
    [TK_GE]        = {NULL,            NULL,           PREC_NONE },
    [TK_SHL]       = {NULL,            expr_binary,    PREC_SHIFT },
    [TK_SHR]       = {NULL,            expr_binary,    PREC_SHIFT },
    [TK_ADD]       = {expr_unary,      expr_binary,    PREC_TERM },
    [TK_SUB]       = {expr_unary,      expr_binary,    PREC_TERM },
    [TK_MUL]       = {NULL,            expr_binary,    PREC_FACTOR },
    [TK_DIV]       = {NULL,            expr_binary,    PREC_FACTOR },
    [TK_MOD]       = {NULL,            expr_binary,    PREC_FACTOR },
    [TK_INC]       = {expr_unary,      NULL,           PREC_NONE },
    [TK_DEC]       = {expr_unary,      NULL,           PREC_NONE },
    [TK_LE_PAREN]  = {NULL,            NULL,           PREC_NONE },
    [TK_RI_PAREN]  = {NULL,            NULL,           PREC_NONE },
    [TK_LE_BRACE]  = {NULL,            NULL,           PREC_NONE },
    [TK_RI_BRACE]  = {NULL,            NULL,           PREC_NONE },
    [TK_LE_BRCKT]  = {NULL,            NULL,           PREC_NONE },
    [TK_RI_BRCKT]  = {NULL,            NULL,           PREC_NONE },
    [TK_COMMA]     = {NULL,            NULL,           PREC_NONE },
    [TK_SEMICOLON] = {NULL,            NULL,           PREC_NONE },
    [TK_COLON]     = {NULL,            NULL,           PREC_NONE },
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

static void expr_number(ParseFunctionArgs) {
    token_t current = prev(sc, ctx);
    emit(ctx, OP_IMM);
    emit(ctx, current.val);
}

static void expr_string(ParseFunctionArgs) {
    emit(ctx, OP_IMM);
    emit(ctx, ctx->heap_cur);
    emit(ctx, OP_STR);
    do{
        token_t current = prev(sc, ctx);
        char c = 0;
        for(int i = 0; i < current.len; i++) {
            c = current.name[i];
            if(c == '\\') { // 处理转义字符
                i++;
                switch(current.name[i]) {
                    case 'n': c = '\n'; break;
                    case 't': c = '\t'; break;
                    case 'r': c = '\r'; break;
                    case '\\': c = '\\'; break;
                    case '\'': c = '\''; break;
                    case '\"': c = '\"'; break;
                    default: c = current.name[i]; break; // 其他字符直接使用
                }
            }
            ctx->heap[ctx->heap_cur] = c;
            ctx->heap_cur++;
        }
    }while(match(ctx, sc, TK_STR));
    ctx->heap[ctx->heap_cur] = '\0';
    ctx->heap_cur++;
}

static void expr_unary(ParseFunctionArgs) {
    token_t current = prev(sc, ctx);
    parse_expr(ctx, sc, PREC_UNARY); // 解析表达式
    switch(current.tk) {
        case TK_ADD:
            // 正号，直接返回
            break;
        case TK_SUB:
            emit(ctx, OP_NEGATE); // 负号，生成取反指令
            break;
        case TK_NOT:
            emit(ctx, OP_NOT); // 逻辑非，生成逻辑非指令
            break;
        case TK_INC:
            emit(ctx, OP_PUSH); // 前置自增，先压栈
            emit(ctx, OP_IMM); // 前置自增
            emit(ctx, 1); // 生成立即数1
            emit(ctx, OP_ADD); // 执行加法
            break;
        case TK_DEC:
            emit(ctx, OP_PUSH); // 前置自减，先压栈
            emit(ctx, OP_IMM); // 前置自减
            emit(ctx, 1); // 生成立即数1
            emit(ctx, OP_SUB); // 执行减法
            break;
        default:
            printf("Unexpected unary operator: %d", current.tk);
            exit(EXIT_FAILURE);
    }
}

static void expr_binary(ParseFunctionArgs) {
    emit(ctx, OP_PUSH);
    token_t current = prev(sc, ctx);
    PrecLv level = Rules[current.tk].prec; // 获取当前操作符的优先级
    parse_expr(ctx, sc, level + 1); // 解析左侧表达式
    switch(current.tk) {
        case TK_ADD:
            emit(ctx, OP_ADD); // 加法
            break;
        case TK_SUB:
            emit(ctx, OP_SUB); // 减法
            break;
        case TK_MUL:
            emit(ctx, OP_MUL); // 乘法
            break;
        case TK_DIV:
            emit(ctx, OP_DIV); // 除法
            break;
        case TK_MOD:
            emit(ctx, OP_MOD); // 取模
            break;
        case TK_SHL:
            emit(ctx, OP_SHL); // 左移
            break;
        case TK_SHR:
            emit(ctx, OP_SHR); // 右移
            break;
        case TK_AND:
            emit(ctx, OP_AND); // 按位与
            break;
        case TK_OR:
            emit(ctx, OP_OR); // 按位或
            break;
        case TK_XOR:
            emit(ctx, OP_XOR); // 按位异或
            break;
    }
}

static void expr_variable(ParseFunctionArgs) {
    token_t tk = prev(sc, ctx);     // 获取当前标识符
    token_t* id = SymFind(ctx, tk); // 在符号表中查找标识符
    if(id->class == 0){
        printf("Identifier not declared before use");
        exit(EXIT_FAILURE);
    }
    if(match(ctx, sc, TK_LE_PAREN)){// 函数调用，不能用查询表，因为无法区分built-in函数和用户定义函数
        if(id->class == TK_FUN) {
            emit(ctx, OP_SAD);   // 自定义函数调用，需要保存当前地址
        }
        int arg_count = 0; // 函数参数计数
        if(!match(ctx, sc, TK_RI_PAREN)) { // 如果不是空参数列表
            do{
                arg_count++;
                parse_expr(ctx, sc, PREC_ASSIGNMENT);
                emit(ctx, OP_PUSH);
            } while(match(ctx, sc, TK_COMMA)); // 处理多个参数
            consume(ctx, sc, TK_RI_PAREN, "Expected ')' after function arguments"); // 确保以右括号结尾
        }
        if(id->class == TK_FUN) {
            emit(ctx, OP_IMM);
            emit(ctx, id->val);     // 函数地址
            emit(ctx, OP_CALL);     // 生成函数调用指令
            emit(ctx, arg_count);   // 使用参数计数
        } else if (id->class == TK_SYS) {
            emit(ctx, id->val); // 使用系统调用的值
            emit(ctx, arg_count); // 使用参数计数
        } else {
            printf("Function call on non-function identifier");
            exit(EXIT_FAILURE);
        }
        return;
    }
    int op_set_code = OP_S_GLO; // 默认操作码为全局变量存储
    int op_get_code = OP_G_GLO;
    int offset = id->val;
    if(id >= ctx->sym_loc) { // 如果是局部变量
        op_set_code = OP_S_LOC; // 设置操作码为局部变量存储
        op_get_code = OP_G_LOC; // 设置操作码为局部变量获取
        offset = id - ctx->sym_loc; // 计算局部变量的偏移量
    }
    if(can_assign && match(ctx, sc, TK_ASSIGN)) {
        parse_expr(ctx, sc, PREC_ASSIGNMENT); // 解析赋值表达式
        emit(ctx, op_set_code); // 生成存储指令
        emit(ctx, offset); // 使用变量的偏移量
    } else {
        emit(ctx, op_get_code); // 生成获取指令
        emit(ctx, offset); // 使用变量的偏移量
    }
}

void parse_expr(context_t ctx, scanner sc, PrecLv level) {
    next(sc, ctx);
    ParseFn prefixFn = Rules[prev(sc, ctx).tk].prefix; 
    if(prefixFn == NULL) {
        printf("Expected expression, but something else found");
        exit(EXIT_FAILURE);
    }
    int can_assign = level <= PREC_ASSIGNMENT; // 是否允许赋值
    prefixFn(PassFunctionArgs);
    while(level <= Rules[prst(sc, ctx).tk].prec) {
        next(sc, ctx);
        ParseFn infixFn = Rules[prev(sc, ctx).tk].infix;
        if(infixFn == NULL) {
            return;
        }
        infixFn(PassFunctionArgs);
    }
}


static void stmt_expr(ParseFunctionArgs) {
    parse_expr(PassFunctionArgs); // 解析表达式
    consume(ctx, sc, TK_SEMICOLON, "Expected ';' after expression statement"); // 确保以分号结尾
}

static void stmt_block(ParseFunctionArgs) {
    while(!match(ctx, sc, TK_RI_BRACE)) { // 解析代码块中的语句
        parse_stmt(ctx, sc);
    }
}

void parse_stmt(context_t ctx, scanner sc) {
    if(match(ctx, sc, TK_IF)) {
        //stmt_if(ctx, sc); // 解析 if 语句
    } else if(match(ctx, sc, TK_WHILE)) {
        //stmt_while(ctx, sc); // 解析 while 语句
    } else if(match(ctx, sc, TK_RETURN)) {
        //stmt_return(ctx, sc); // 解析 return 语句
    } else if(match(ctx, sc, '{')) {
        stmt_block(ctx, sc, 1); // 解析代码块
    } else if(match(ctx, sc, TK_INT) || match(ctx, sc, TK_CHAR) || match(ctx, sc, TK_VOID)) {
        //stmt_decl(ctx, sc); // 解析变量声明
    } else {
        stmt_expr(ctx, sc, 1); // 解析表达式语句
    }
    printf("end of statement\n");
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
    if(match(ctx, sc, TK_LE_PAREN)){    // 函数声明
        printf("Function declaration found\n");
        id->class = TK_FUN;
        id->val = ctx->btcode_cur - ctx->btcode; // 函数地址为当前字节码位置
        SymSetloc(ctx);
        int arg_count = 0;
        while(!match(ctx, sc, TK_RI_PAREN)) { // 解析函数参数
            if(arg_count > 0) {
                consume(ctx, sc, TK_COMMA, "Expected ',' in function argument list");
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
        consume(ctx, sc, TK_LE_BRACE, "Expected '{' after function declaration");
        stmt_block(ctx, sc, 1);
        SymEndloc(ctx);
        emit(ctx, OP_IMM);  // 配置默认返回值
        emit(ctx, 0);
        emit(ctx, OP_RET);
    } else {      
        id->class = TK_GLO; 
        id->val = id - ctx->sym;
        if(match(ctx, sc, TK_ASSIGN)) {
            parse_expr(ctx, sc, PREC_ASSIGNMENT);
        } else {
            emit(ctx, OP_IMM); // 初始化为0
            emit(ctx, 0);
        }
        emit(ctx, OP_S_GLO); // 存储全局变量值
        emit(ctx, id->val);   // 使用全局变量的偏移量
        consume(ctx, sc, TK_SEMICOLON, "Expected ';' after global variable declaration"); // 暂时不处理多个变量声明
    }
}


void compile(context_t ctx, scanner sc) {
    next(sc, ctx); // 开始解析，扫描第一个token
    while(!match(ctx, sc, 0)) {
        parse_global(ctx, sc);
    }
}
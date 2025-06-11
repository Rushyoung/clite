#include "parser.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "def.h"
#include "native.h"
#include "opcode.h"

#include "scanner.h"
#include "token.h"

#define raise(l, ...) ({ \
    printf("line: %lld\n", (l)); \
    printf("    : "); \
    printf(__VA_ARGS__); \
    printf("\n"); \
    exit(EXIT_FAILURE); \
})

// 定义解析函数
static void expr_number(ParseFunctionArgs);
static void expr_unary(ParseFunctionArgs);
static void expr_binary(ParseFunctionArgs);
static void expr_variable(ParseFunctionArgs);
static void expr_string(ParseFunctionArgs);
static void expr_and(ParseFunctionArgs);
static void expr_or(ParseFunctionArgs);
static void expr_preinc(ParseFunctionArgs);
static void expr_call(ParseFunctionArgs);
static void expr_offset(ParseFunctionArgs);
static void expr_list(ParseFunctionArgs);

static void stmt_expr(ParseFunctionArgs);
static void stmt_block(ParseFunctionArgs);
static void stmt_return(ParseFunctionArgs);
static void stmt_decl(ParseFunctionArgs);
static void stmt_while(ParseFunctionArgs);
static void stmt_if(ParseFunctionArgs);
static void stmt_for(ParseFunctionArgs);

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
    [TK_LOR]       = {NULL,            expr_or,        PREC_OR },
    [TK_LAN]       = {NULL,            expr_and,       PREC_AND },
    [TK_NOT]       = {expr_unary,      NULL,           PREC_NONE },
    [TK_OR]        = {NULL,            expr_binary,    PREC_BITWISE },
    [TK_XOR]       = {NULL,            expr_binary,    PREC_BITWISE },
    [TK_AND]       = {NULL,            expr_binary,    PREC_BITWISE },
    [TK_EQ]        = {NULL,            expr_binary,    PREC_EQUALITY },
    [TK_NE]        = {NULL,            expr_binary,    PREC_EQUALITY },
    [TK_LT]        = {NULL,            expr_binary,    PREC_COMPARISON },
    [TK_GT]        = {NULL,            expr_binary,    PREC_COMPARISON },
    [TK_LE]        = {NULL,            expr_binary,    PREC_COMPARISON },
    [TK_GE]        = {NULL,            expr_binary,    PREC_COMPARISON },
    [TK_SHL]       = {NULL,            expr_binary,    PREC_SHIFT },
    [TK_SHR]       = {NULL,            expr_binary,    PREC_SHIFT },
    [TK_ADD]       = {expr_unary,      expr_binary,    PREC_TERM },
    [TK_SUB]       = {expr_unary,      expr_binary,    PREC_TERM },
    [TK_MUL]       = {NULL,            expr_binary,    PREC_FACTOR },
    [TK_DIV]       = {NULL,            expr_binary,    PREC_FACTOR },
    [TK_MOD]       = {NULL,            expr_binary,    PREC_FACTOR },
    [TK_INC]       = {expr_preinc,     NULL,           PREC_NONE },
    [TK_DEC]       = {expr_preinc,     NULL,           PREC_NONE },
    [TK_LE_PAREN]  = {NULL,            expr_call,      PREC_CALL },
    [TK_RI_PAREN]  = {NULL,            NULL,           PREC_NONE },
    [TK_LE_BRACE]  = {expr_list,       NULL,           PREC_NONE },
    [TK_RI_BRACE]  = {NULL,            NULL,           PREC_NONE },
    [TK_LE_BRCKT]  = {NULL,            expr_offset,    PREC_OFFSET },
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

static void expect(context_t ctx, scanner sc, TkType tk, char* msg) {
    token_t current = prst(sc, ctx);
    if(current.tk != tk) {
        raise(sc->line, msg);
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
            raise(sc->line, "Expected type declaration (int, char, void)");
    }
    next(sc, ctx); // 跳过类型声明
    return type; // 返回解析的类型
}

static token_t* __identifier(context_t ctx, scanner sc, int* type) {
    while(match(ctx, sc, TK_MUL)) {
        *type += TP_PTR; // 处理指针类型
    }
    expect(ctx, sc, TK_ID, "Expected identifier after type declaration");
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
        default:
            raise(sc->line, "Unexpected unary operator");
    }
}

static void expr_preinc(ParseFunctionArgs) {
    token_t current = prev(sc, ctx);
    expect(ctx, sc, TK_ID, "Expected identifier after increment/decrement operator");
    token_t* id = SymFind(ctx, prev(sc, ctx));
    if(id->class == 0) {
        raise(sc->line, "Identifier not declared before use");
    }
    uint64_t op_set = OP_S_GLO;
    uint64_t op_get = OP_G_GLO;
    uint64_t op_calc = 0;
    if(id >= ctx->sym_loc) { // 如果是局部变量
        op_set = OP_S_LOC;
        op_get = OP_G_LOC;
    }
    if(current.tk == TK_INC) {
        op_calc = OP_ADD; // 自增
    } else if(current.tk == TK_DEC) {
        op_calc = OP_SUB; // 自减
    }
    emit(ctx, op_get);
    emit(ctx, id->val);
    emit(ctx, OP_PUSH);
    emit(ctx, OP_IMM);
    emit(ctx, 1);
    emit(ctx, op_calc);
    emit(ctx, op_set);
    emit(ctx, id->val);
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
        case TK_EQ:
            emit(ctx, OP_EQU); // 等于
            break;
        case TK_NE:
            emit(ctx, OP_EQU);
            emit(ctx, OP_NOT);
            break; // 不等于
        case TK_LT:
            emit(ctx, OP_LES);
            break;
        case TK_GT:
            emit(ctx, OP_GRT);
            break;
        case TK_LE:
            emit(ctx, OP_GRT);
            emit(ctx, OP_NOT);
            break;
        case TK_GE:
            emit(ctx, OP_LES);
            emit(ctx, OP_NOT);
            break;
    }
}

static void expr_and(ParseFunctionArgs) {
    emit(ctx, OP_JZ);
    uint64_t* addr = blank(ctx); // 留白，跳转地址
    parse_expr(ctx, sc, PREC_AND);
    patch(ctx, addr, ctx->btcode_cur - ctx->btcode); // 填充跳转地址
}

static void expr_or(ParseFunctionArgs) {
    emit(ctx, OP_JZ);
    uint64_t* addr_else = blank(ctx); // 留白，跳转到 else 分支
    emit(ctx, OP_JMP);
    uint64_t* addr_end = blank(ctx);  // 留白，跳转到结束
    patch(ctx, addr_else, ctx->btcode_cur - ctx->btcode); // 填充 else 分支跳转地址
    parse_expr(ctx, sc, PREC_OR); // 解析右侧表达式
    patch(ctx, addr_end, ctx->btcode_cur - ctx->btcode); // 填充结束跳转地址
}

static void expr_variable(ParseFunctionArgs) {
    token_t tk = prev(sc, ctx);     // 获取当前标识符
    token_t* id = SymFind(ctx, tk); // 在符号表中查找标识符
    if(id->class == 0){
        raise(sc->line, "Identifier not declared before use");
    }
    if(id->class == TK_FUN || id->class == TK_SYS) {
        emit(ctx, OP_IMM);
        emit(ctx, id->val); // 函数地址
        return;
    }
    int op_set_code = OP_S_GLO; // 默认操作码为全局变量存储
    int op_get_code = OP_G_GLO;
    if(id >= ctx->sym_loc) { // 如果是局部变量
        op_set_code = OP_S_LOC; // 设置操作码为局部变量存储
        op_get_code = OP_G_LOC; // 设置操作码为局部变量获取
    }
    if(match(ctx, sc, TK_INC) || match(ctx, sc, TK_DEC)) {
        token_t inc_dec = prev(sc, ctx); // 获取自增或自减操作符
        uint64_t op_calc = (inc_dec.tk == TK_INC) ? OP_ADD : OP_SUB;
        emit(ctx, op_get_code);
        emit(ctx, id->val);
        emit(ctx, OP_PUSH);
        emit(ctx, OP_PUSH);
        emit(ctx, OP_IMM);
        emit(ctx, 1);
        emit(ctx, op_calc);
        emit(ctx, op_set_code);
        emit(ctx, id->val);
        emit(ctx, OP_IMM);
        emit(ctx, 0);
        emit(ctx, OP_ADD);
        return;
    }
    if(can_assign && match(ctx, sc, TK_ASSIGN)) {
        parse_expr(ctx, sc, PREC_ASSIGNMENT); // 解析赋值表达式
        emit(ctx, op_set_code);
    } else {
        emit(ctx, op_get_code);
    }
    emit(ctx, id->val);
}

static void expr_call(ParseFunctionArgs) {
    emit(ctx, OP_SAD);
    emit(ctx, OP_PUSH);
    int arg_count = 0;  // 函数参数计数
    if(!match(ctx, sc, TK_RI_PAREN)) { // 如果不是空参数列表
        do{
            arg_count++;
            parse_expr(ctx, sc, PREC_ASSIGNMENT);
            emit(ctx, OP_PUSH);
        } while(match(ctx, sc, TK_COMMA)); // 处理多个参数
        expect(ctx, sc, TK_RI_PAREN, "Expected ')' after function arguments"); // 确保以右括号结尾
    }
    emit(ctx, OP_CALL);     // 生成函数调用指令
    emit(ctx, arg_count);   // 使用参数计数
}

static void expr_offset(ParseFunctionArgs) {
    emit(ctx, OP_PUSH);
    parse_expr(ctx, sc, PREC_OFFSET); // 解析偏移表达式
    if(!match(ctx, sc, TK_RI_BRCKT)) {
        raise(sc->line, "Expected ']' after array offset");
    }
    emit(ctx, OP_OFFSET); // 生成数组偏移指令
}

static void expr_list(ParseFunctionArgs) {
    if(match(ctx, sc, TK_RI_BRACE)) {
        emit(ctx, OP_IMM);
        emit(ctx, 0); // 空列表
        return;
    }
    emit(ctx, OP_IMM);
    emit(ctx, (uint64_t)buildin_list); // 使用内置列表函数
    emit(ctx, OP_SAD);
    emit(ctx, OP_PUSH);
    int member_count = 0;
    do{
        member_count++;
        parse_expr(ctx, sc, PREC_ASSIGNMENT); // 解析列表成员
        emit(ctx, OP_PUSH); // 将成员压入栈中
    }while(match(ctx, sc, TK_COMMA)); // 处理多个成员
    expect(ctx, sc, TK_RI_BRACE, "Expected '}' after list"); // 确保以右大括号结尾
    emit(ctx, OP_CALL);
    emit(ctx, member_count); // 使用成员计数
}

void parse_expr(context_t ctx, scanner sc, PrecLv level) {
    next(sc, ctx);
    ParseFn prefixFn = Rules[prev(sc, ctx).tk].prefix; 
    if(prefixFn == NULL) {
        raise(sc->line, "Expected expression, but something else found");
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
    expect(ctx, sc, TK_SEMICOLON, "Expected ';' after expression statement"); // 确保以分号结尾
}

static void stmt_block(ParseFunctionArgs) {
    while(!match(ctx, sc, TK_RI_BRACE)) { // 解析代码块中的语句
        parse_stmt(ctx, sc);
    }
}

static void stmt_return(ParseFunctionArgs) {
    if(match(ctx, sc, TK_SEMICOLON)) {
        emit(ctx, OP_IMM);
        emit(ctx, 0);
    } else {
        parse_expr(ctx, sc, PREC_ASSIGNMENT);
        expect(ctx, sc, TK_SEMICOLON, "Expected ';' after return statement");
    }
    emit(ctx, OP_RET);
}

static void stmt_decl(ParseFunctionArgs) {
    int base_type = 0;
    token_t type = prev(sc, ctx); // 获取当前类型声明
    if(type.tk == TK_INT) {
        base_type = TP_INT; // 整型
    } else if(type.tk == TK_CHAR) {
        base_type = TP_CHAR; // 字符型
    } else if(type.tk == TK_VOID) {
        base_type = TP_VOID; // 空类型
    } else {
        raise(sc->line, "Expected type declaration (int, char, void)");
    }
    int real_type = base_type;
    do{
        token_t* id = __identifier(ctx, sc, &real_type); // 解析标识符
        if(id->class == TK_LOC) {
            raise(sc->line, "Variable '%.*s' already defined", id->name, id->len);
        } else if(id->class == TK_GLO || id->class == TK_FUN) {
            id = SymAdd(ctx, *id); // 如果是全局变量或函数，则添加到符号表
        }
        if(real_type == TP_VOID) {
            raise(sc->line, "Variable '%.*s' cannot be of type void", id->name, id->len);
        }
        id->type = real_type;
        id->class = TK_LOC;
        id->val = id - ctx->sym_loc;
        if(match(ctx, sc, TK_ASSIGN)) {
            parse_expr(ctx, sc, PREC_ASSIGNMENT);
        } else {
            emit(ctx, OP_IMM);
            emit(ctx, 0);
        }
        emit(ctx, OP_S_LOC); 
        emit(ctx, id->val);
    }while(match(ctx, sc, TK_COMMA));
    expect(ctx, sc, TK_SEMICOLON, "Expected ';' after variable declaration"); // 确保以分号结尾
}

static void stmt_while(ParseFunctionArgs) {
    expect(ctx, sc, TK_LE_PAREN, "Expected '(' after 'while'");
    uint64_t* addr_start = ctx->btcode_cur;
    parse_expr(ctx, sc, PREC_ASSIGNMENT);
    expect(ctx, sc, TK_RI_PAREN, "Expected ')' after 'while' condition");
    emit(ctx, OP_JZ);
    uint64_t* addr_end = blank(ctx);
    parse_stmt(ctx, sc);
    emit(ctx, OP_JMP);
    emit(ctx, addr_start - ctx->btcode);
    patch(ctx, addr_end, ctx->btcode_cur - ctx->btcode);
}

static void stmt_for(ParseFunctionArgs) {
    expect(ctx, sc, TK_LE_PAREN, "Expected '(' after 'for'");
    if(match(ctx, sc, TK_SEMICOLON)) {
    } else if(match(ctx, sc, TK_INT) || match(ctx, sc, TK_CHAR) || match(ctx, sc, TK_VOID)) {
        stmt_decl(ctx, sc, 1); // 解析 for 循环的初始化部分
    } else {
        stmt_expr(ctx, sc, 1); // 解析 for 循环的初始化表达式
    }
    uint64_t* addr_start = ctx->btcode_cur;
    uint64_t* addr_end = NULL;
    if(!match(ctx, sc, TK_SEMICOLON)) {
        parse_expr(ctx, sc, PREC_ASSIGNMENT); // 解析条件表达式
        expect(ctx, sc, TK_SEMICOLON, "Expected ';' after 'for' condition");
        emit(ctx, OP_JZ);
        addr_end = blank(ctx); // 留白，跳转到循环结束
    }
    if(!match(ctx, sc, TK_RI_PAREN)) {
        emit(ctx, OP_JMP);
        uint64_t* addr_body = blank(ctx);
        uint64_t* addr_inc = ctx->btcode_cur;
        parse_expr(ctx, sc, PREC_ASSIGNMENT);
        expect(ctx, sc, TK_RI_PAREN, "Expected ')' after 'for' increment expression");
        emit(ctx, OP_JMP);
        emit(ctx, addr_start - ctx->btcode);
        addr_start = addr_inc;
        patch(ctx, addr_body, ctx->btcode_cur - ctx->btcode);
    }
    parse_stmt(ctx, sc);
    emit(ctx, OP_JMP);
    emit(ctx, addr_start - ctx->btcode); // 跳转到循环开始
    if(addr_end) {
        patch(ctx, addr_end, ctx->btcode_cur - ctx->btcode);
    }
}

static void stmt_if(ParseFunctionArgs){
    expect(ctx, sc, TK_LE_PAREN, "Expected '(' after 'if'");
    parse_expr(ctx, sc, PREC_ASSIGNMENT);
    expect(ctx, sc, TK_RI_PAREN, "Expected ')' after 'if' condition");
    emit(ctx, OP_JZ);
    uint64_t* if_end = blank(ctx);
    parse_stmt(ctx, sc);
    //if end
    if(match(ctx, sc, TK_ELSE)){
        emit(ctx, OP_JMP);
        uint64_t* el_end = blank(ctx);
        patch(ctx, if_end, ctx->btcode_cur - ctx->btcode);
        parse_stmt(ctx, sc);
        patch(ctx, el_end, ctx->btcode_cur - ctx->btcode);
    } else {
        patch(ctx, if_end, ctx->btcode_cur - ctx->btcode);
    }
}

void parse_stmt(context_t ctx, scanner sc) {
    if(match(ctx, sc, TK_IF)) {
        stmt_if(ctx, sc, 1); // 解析 if 语句
    } else if(match(ctx, sc, TK_WHILE)) {
        stmt_while(ctx, sc, 1); // 解析 while 语句
    } else if(match(ctx, sc, TK_FOR)) {
        stmt_for(ctx, sc, 1); // 解析 for 语句
    } else if(match(ctx, sc, TK_RETURN)) {
        stmt_return(ctx, sc, 1);
    } else if(match(ctx, sc, TK_LE_BRACE)) {
        stmt_block(ctx, sc, 1);
    } else if(match(ctx, sc, TK_INT) || match(ctx, sc, TK_CHAR) || match(ctx, sc, TK_VOID)) {
        stmt_decl(ctx, sc, 1);
    } else {
        stmt_expr(ctx, sc, 1); // 解析表达式语句
    }
}

void parse_global(context_t ctx, scanner sc) {
    int base_type = TP_INT; // 基本类型，默认为整型
    int real_type = TP_INT; // 实际类型，默认为整型
    real_type = base_type = __ctype(ctx, sc); // 解析类型声明
    token_t* id = __identifier(ctx, sc, &real_type); // 解析标识符
    if(id->class == TK_GLO || id->class == TK_FUN) {
        raise(sc->line, "Global variable '%.*s' already defined", id->name, id->len);
    }
    id->type = real_type; // 设置变量类型
    if(match(ctx, sc, TK_LE_PAREN)){    // 函数声明
        emit(ctx, OP_JMP);
        uint64_t* addr = blank(ctx); // 留白，函数结束地址
        id->class = TK_FUN;
        id->val = ctx->btcode_cur - ctx->btcode; // 函数地址为当前字节码位置
        emit(ctx, OP_FUNC); // 函数入口标记
        SymSetloc(ctx);
        int arg_count = 0;
        while(!match(ctx, sc, TK_RI_PAREN)) { // 解析函数参数
            if(arg_count > 0) {
                expect(ctx, sc, TK_COMMA, "Expected ',' in function argument list");
            }
            int arg_type = __ctype(ctx, sc); // 解析参数类型
            token_t* arg_id = __identifier(ctx, sc, &arg_type); // 解析参数标识符
            if(arg_id->class == TK_LOC) {
                raise(sc->line, "Function argument '%.*s' already defined in this scope", arg_id->name, arg_id->len);
            } else if(arg_id->class == TK_GLO || arg_id->class == TK_FUN) {
                arg_id = SymAdd(ctx, *arg_id); // 如果是全局变量或函数，则添加到符号表
            }
            arg_id->type = arg_type; // 设置参数类型
            arg_id->class = TK_LOC; // 设置为局部变量
            arg_id->val = arg_id - ctx->sym_loc; // 计算局部变量的偏移量
            arg_count++;
        }
        expect(ctx, sc, TK_LE_BRACE, "Expected '{' after function declaration");
        stmt_block(ctx, sc, 1);
        SymEndloc(ctx);
        emit(ctx, OP_IMM);  // 配置默认返回值
        emit(ctx, 0);
        emit(ctx, OP_RET);
        patch(ctx, addr, ctx->btcode_cur - ctx->btcode); // 填充函数结束地址
    } else {
        define_loop:
        id->class = TK_GLO; 
        id->val = id - ctx->sym;
        if(match(ctx, sc, TK_ASSIGN)) {
            parse_expr(ctx, sc, PREC_ASSIGNMENT);
        } else {
            emit(ctx, OP_IMM); // 初始化为0
            emit(ctx, 0);
        }
        emit(ctx, OP_S_GLO);
        emit(ctx, id->val);   // 使用全局变量的偏移量
        if(match(ctx, sc, TK_SEMICOLON)){
            return;
        } else if(!match(ctx, sc, TK_COMMA)) {
            raise(sc->line, "Expected ',' or ';' after global variable declaration");
        }
        id = __identifier(ctx, sc, &real_type); // 继续解析下一个标识符
        if(id->class == TK_GLO || id->class == TK_FUN) {
            raise(sc->line, "Global variable '%.*s' already defined", id->name, id->len);
        }
        id->type = real_type; // 设置变量类型
        goto define_loop;
    }
}


void compile(context_t ctx, scanner sc) {
    next(sc, ctx); // 开始解析，扫描第一个token
    while(!match(ctx, sc, 0)) {
        parse_global(ctx, sc);// 全局声明
    }

    if(ctx->sym[ctx->main_id].class != TK_FUN) {
        raise(sc->line, "Main function not defined");
    }

    emit(ctx, OP_JMP);
    emit(ctx, ctx->sym[ctx->main_id].val);
}
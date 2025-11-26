#include "parser.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"
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
static void expr_group(ParseFunctionArgs);
static void expr_variable(ParseFunctionArgs);
static void expr_string(ParseFunctionArgs);
static void expr_and(ParseFunctionArgs);
static void expr_or(ParseFunctionArgs);
static void expr_preinc(ParseFunctionArgs);
static void expr_call(ParseFunctionArgs);
static void expr_offset(ParseFunctionArgs);
static void expr_list(ParseFunctionArgs);
static void expr_assign(ParseFunctionArgs);
static void expr_ternary(ParseFunctionArgs);
static void expr_sizeof(ParseFunctionArgs);

static void stmt_expr(ParseFunctionArgs);
static void stmt_block(ParseFunctionArgs);
static void stmt_return(ParseFunctionArgs);
static void stmt_decl(ParseFunctionArgs);
static void stmt_while(ParseFunctionArgs);
static void stmt_if(ParseFunctionArgs);
static void stmt_for(ParseFunctionArgs);
static void stmt_dowhile(ParseFunctionArgs);
static void stmt_break(ParseFunctionArgs);
static void stmt_continue(ParseFunctionArgs);

static void parse_expr(context_t ctx, scanner sc, PrecLv level);
static void parse_stmt(context_t ctx, scanner sc);
static void parse_global(context_t ctx, scanner sc);
static void parse_enum(context_t ctx, scanner sc);
static void parse_function(context_t ctx, scanner sc, token_t* name, int need_jit);
static void parse_assign(context_t ctx, scanner sc, token_t* id);

ParseRule Rules[] = {//infix,          prefix,         precedence
    [TK_NUM]       = {expr_number,     NULL,           PREC_NONE },
    [TK_FUN]       = {NULL,            NULL,           PREC_NONE },
    [TK_SYS]       = {NULL,            NULL,           PREC_NONE },
    [TK_GLO]       = {NULL,            NULL,           PREC_NONE },
    [TK_LOC]       = {NULL,            NULL,           PREC_NONE },
    [TK_ID]        = {expr_variable,   NULL,           PREC_NONE },
    [TK_STR]       = {expr_string,     NULL,           PREC_NONE },
    [TK_BREAK]     = {NULL,            NULL,           PREC_NONE },
    [TK_CHAR]      = {NULL,            NULL,           PREC_NONE },
    [TK_ELSE]      = {NULL,            NULL,           PREC_NONE },
    [TK_ENUM]      = {NULL,            NULL,           PREC_NONE },
    [TK_IF]        = {NULL,            NULL,           PREC_NONE },
    [TK_INT]       = {NULL,            NULL,           PREC_NONE },
    [TK_RETURN]    = {NULL,            NULL,           PREC_NONE },
    [TK_SIZEOF]    = {expr_sizeof,     NULL,           PREC_NONE },
    [TK_WHILE]     = {NULL,            NULL,           PREC_NONE },
    [TK_VOID]      = {NULL,            NULL,           PREC_NONE },
    [TK_FLOAT]     = {expr_number,     NULL,           PREC_NONE },
    [TK_ASSIGN]    = {NULL,            expr_assign,    PREC_ASSIGNMENT },
    [TK_COND]      = {NULL,            expr_ternary,   PREC_TERNARY },
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
    [TK_LE_PAREN]  = {expr_group,      expr_call,      PREC_CALL },
    [TK_RI_PAREN]  = {NULL,            NULL,           PREC_NONE },
    [TK_LE_BRACE]  = {expr_list,       NULL,           PREC_NONE },
    [TK_RI_BRACE]  = {NULL,            NULL,           PREC_NONE },
    [TK_LE_BRCKT]  = {NULL,            expr_offset,    PREC_OFFSET },
    [TK_RI_BRCKT]  = {NULL,            NULL,           PREC_NONE },
    [TK_COMMA]     = {NULL,            NULL,           PREC_NONE },
    [TK_SEMICOLON] = {NULL,            NULL,           PREC_NONE },
    [TK_COLON]     = {NULL,            NULL,           PREC_NONE },
};

// 检查当前 token 是否匹配指定类型，如果匹配则前进到下一个 token
static int match(context_t ctx, scanner sc, TkType tk) {
    token_t current = prst(sc);
    if(current.tk == tk) {
        next(sc, ctx);
        return 1; // 匹配成功
    }
    return 0; // 匹配失败
}

// 期望当前 token 为指定类型，否则报错并退出
static void expect(context_t ctx, scanner sc, TkType tk, char* msg) {
    token_t current = prst(sc);
    if(current.tk != tk) {
        raise(sc->line, msg);
    }
    next(sc, ctx); // 跳过匹配的 token
}

// 解析标识符，并处理指针类型
static token_t* __identifier(context_t ctx, scanner sc, int* type) {
    while(match(ctx, sc, TK_MUL)) {
        *type += TYPE_PTR; // 处理指针类型
    }
    expect(ctx, sc, TK_ID, "Expected identifier after type declaration");
    return SymFind(ctx, prev(sc));
}

// 解析类型声明
static int match_type(context_t ctx, scanner sc) {
    if(match(ctx, sc, TK_INT)){
        ctx->expr_type = TYPE_INT;
    } else if(match(ctx, sc, TK_CHAR)){
        ctx->expr_type = TYPE_CHAR;
    } else if(match(ctx, sc, TK_VOID)){
        ctx->expr_type = TYPE_VOID;
    } else if(match(ctx, sc, TK_FLOAT)){
        ctx->expr_type = TYPE_FLOAT;
    } else {
        return 0; // 没有匹配到类型
    }
    return 1;
}

// 解析数字常量表达式
static void expr_number(ParseFunctionArgs) {
    token_t current = prev(sc);
    emit(ctx, OP_IMM);
    emit(ctx, current.val.uval);
    if(current.type == TYPE_FLOAT) {
        ctx->expr_type = TYPE_FLOAT;
    } else {
        ctx->expr_type = TYPE_INT;
    }
}

// 解析括号表达式
static void expr_group(ParseFunctionArgs) {
    parse_expr(ctx, sc, PREC_ASSIGNMENT);
    if(!match(ctx, sc, TK_RI_PAREN)) {
        raise(sc->line, "Expected ')' after expression");
    }
}

// 解析字符串字面量表达式
static void expr_string(ParseFunctionArgs) {
    emit(ctx, OP_IMM);
    emit(ctx, ctx->heap_cur + (uint64_t)ctx->heap); // 字符串在堆中的地址
    do{
        token_t current = prev(sc);
        char c = 0;
        for(size_t i = 0; i < current.len; i++) {
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
    ctx->expr_type = TYPE_CHAR + TYPE_PTR; // 字符串类型为 char* (指针类型)
}

// 解析赋值表达式 (目前实现为报错)
static void expr_assign(ParseFunctionArgs) {
    raise(sc->line, "The left side must be a variable");
}

// 解析一元表达式
static void expr_unary(ParseFunctionArgs) {
    token_t current = prev(sc);
    parse_expr(ctx, sc, PREC_UNARY); // 解析表达式
    switch(current.tk) {
        case TK_ADD:
            break;
        case TK_SUB:
            emit(ctx, OP_PUSH);
            emit(ctx, OP_IMM);
            emit(ctx, -1);
            if(ctx->expr_type == TYPE_FLOAT){
                emit(ctx, OP_FLT);
                emit(ctx, OP_MUL_F);
            } else {
                emit(ctx, OP_MUL);
            }
            break;
        case TK_NOT:
            emit(ctx, OP_NOT); // 逻辑非，生成逻辑非指令
            break;
        default:
            raise(sc->line, "Unexpected unary operator");
    }
}

// 解析前置自增/自减表达式
static void expr_preinc(ParseFunctionArgs) {
    token_t current = prev(sc);
    expect(ctx, sc, TK_ID, "Expected identifier after increment/decrement operator");
    token_t* id = SymFind(ctx, prev(sc));
    if(id->klass == 0) {
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
    if(id->type == TYPE_FLOAT){
        op_calc += (OP_ADD_F - OP_ADD);
    }
    emit(ctx, op_get);
    emit(ctx, id->val.uval);
    emit(ctx, OP_PUSH);
    emit(ctx, OP_IMM);
    emit(ctx, 1);
    if(id->type == TYPE_FLOAT) {
        emit(ctx, OP_FLT);
    }
    emit(ctx, op_calc);
    emit(ctx, op_set);
    emit(ctx, id->val.uval);
    ctx->expr_type = id->type;
}

// 解析二元表达式
static void expr_binary(ParseFunctionArgs) {
    int type_left = ctx->expr_type;
    emit(ctx, OP_PUSH);
    token_t current = prev(sc);
    PrecLv level = Rules[current.tk].prec; // 获取当前操作符的优先级
    parse_expr(ctx, sc, level + 1); // 解析右侧表达式
    int type_right = ctx->expr_type;    // 获取右侧表达式的类型
    int calc_flt = (type_left == TYPE_FLOAT || type_right == TYPE_FLOAT) ? (OP_ADD_F - OP_ADD) : 0; // 检查是否有浮点数参与运算
    if(calc_flt && ( type_left != TYPE_FLOAT || type_right != TYPE_FLOAT)){
        raise(sc->line, "Cannot calculate between float and int directly");
    }
    switch(current.tk) {
        case TK_ADD:
            emit(ctx, OP_ADD + calc_flt); // 加法
            break;
        case TK_SUB:
            emit(ctx, OP_SUB + calc_flt); // 减法
            break;
        case TK_MUL:
            emit(ctx, OP_MUL + calc_flt); // 乘法
            break;
        case TK_DIV:
            emit(ctx, OP_DIV + calc_flt); // 除法
            break;
        case TK_MOD:
            if(calc_flt){
                raise(sc->line, "Cannot use modulus operator on float type");
            }
            emit(ctx, OP_MOD); // 取模
            break;
        case TK_SHL:
            emit(ctx, OP_SHL); // 左移
            break;
        case TK_SHR:
            emit(ctx, OP_SHR); // 右移
            break;
        case TK_AND:
            if(calc_flt){
                raise(sc->line, "Cannot use bitwise AND operator on float type");
            }
            emit(ctx, OP_AND); // 按位与
            break;
        case TK_OR:
            if(calc_flt){
                raise(sc->line, "Cannot use bitwise OR operator on float type");
            }
            emit(ctx, OP_OR); // 按位或
            break;
        case TK_XOR:
            if(calc_flt){
                raise(sc->line, "Cannot use bitwise XOR operator on float type");
            }
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
    ctx->expr_type = calc_flt ? TYPE_FLOAT : TYPE_INT; // 设置表达式类型
}

// 解析逻辑与 (&&) 表达式
static void expr_and(ParseFunctionArgs) {
    emit(ctx, OP_JZ);
    uint64_t* addr = blank(ctx); // 留白，跳转地址
    parse_expr(ctx, sc, PREC_AND);
    patch(ctx, addr, ctx->btcode_cur - ctx->btcode); // 填充跳转地址
}

// 解析逻辑或 (||) 表达式
static void expr_or(ParseFunctionArgs) {
    emit(ctx, OP_JZ);
    uint64_t* addr_else = blank(ctx); // 留白，跳转到 else 分支
    emit(ctx, OP_JMP);
    uint64_t* addr_end = blank(ctx);  // 留白，跳转到结束
    patch(ctx, addr_else, ctx->btcode_cur - ctx->btcode); // 填充 else 分支跳转地址
    parse_expr(ctx, sc, PREC_OR); // 解析右侧表达式
    patch(ctx, addr_end, ctx->btcode_cur - ctx->btcode); // 填充结束跳转地址
}

// 解析变量表达式，处理变量读取、赋值和后置自增/自减
static void expr_variable(ParseFunctionArgs) {
    token_t tk = prev(sc);     // 获取当前标识符
    token_t* id = SymFind(ctx, tk); // 在符号表中查找标识符
    if(id->klass == 0){
        raise(sc->line, "Identifier not declared before use");
    }
    ctx->expr_type = id->type;
    if(id->klass == TK_FUN || id->klass == TK_SYS) {
        emit(ctx, OP_IMM);
        emit(ctx, id->val.uval); // 函数地址
        return;
    }
    int op_set_code = OP_S_GLO; // 默认操作码为全局变量存储
    int op_get_code = OP_G_GLO;
    if(id >= ctx->sym_loc) { // 如果是局部变量
        op_set_code = OP_S_LOC; // 设置操作码为局部变量存储
        op_get_code = OP_G_LOC; // 设置操作码为局部变量获取
    }
    if(match(ctx, sc, TK_INC) || match(ctx, sc, TK_DEC)) {
        token_t inc_dec = prev(sc); // 获取自增或自减操作符
        uint64_t op_calc = (inc_dec.tk == TK_INC) ? OP_ADD : OP_SUB;
        uint64_t op_calc_f = (id->type == TYPE_FLOAT) ? (OP_ADD_F - OP_ADD) : 0;
        emit(ctx, op_get_code);
        emit(ctx, id->val.uval);
        emit(ctx, OP_PUSH);
        emit(ctx, OP_PUSH);
        emit(ctx, OP_IMM);
        emit(ctx, 1);
        if(op_calc_f){
            emit(ctx, OP_FLT);
        }
        emit(ctx, op_calc + op_calc_f);
        emit(ctx, op_set_code);
        emit(ctx, id->val.uval);
        emit(ctx, OP_POP);
        return;
    }
    if(can_assign && match(ctx, sc, TK_ASSIGN)) {
        parse_expr(ctx, sc, PREC_ASSIGNMENT); // 解析赋值表达式
        emit(ctx, op_set_code);
    } else {
        emit(ctx, op_get_code);
    }
    emit(ctx, id->val.uval);
}

static void expr_sizeof(ParseFunctionArgs) { // to fix bug
    emit(ctx, OP_IMM);
    expect(ctx, sc, TK_LE_PAREN, "Expected '(' after 'sizeof'");
    if(match(ctx, sc, TK_ID)){
        token_t* id = SymFind(ctx, prev(sc));
        if(id->klass == 0) {
            raise(sc->line, "Identifier not declared before use");
        }
        if(id->klass == TK_FUN){
            emit(ctx, 8);
        } else {
            emit(ctx, (id->type == TYPE_VOID || id->type == TYPE_CHAR) ? 1 : 8); // 函数或指针类型为 8 字节，其他类型为 1 字节
        }
    } else {
        expect(ctx, sc, TK_INT, "Expected type after 'sizeof'");
        int type = TYPE_VOID; // 解析类型 @todo
        if(type == TYPE_VOID) {
            raise(sc->line, "Cannot use sizeof on void type");
        }
        emit(ctx, (type == TYPE_CHAR || type == TYPE_VOID) ? 1 : 8); // char 和 void 类型为 1 字节，其他类型为 8 字节
    }
    expect(ctx, sc, TK_RI_PAREN, "Expected ')' after 'sizeof' expression");
}

// 解析函数调用表达式
static void expr_call(ParseFunctionArgs) {
    emit(ctx, OP_PUSH);
    emit(ctx, OP_PUSH); // 至少需要栈的两个位置
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

// 解析数组下标/偏移量表达式
static void expr_offset(ParseFunctionArgs) {
    emit(ctx, OP_PUSH);
    parse_expr(ctx, sc, PREC_ASSIGNMENT); // 解析偏移表达式
    if(!match(ctx, sc, TK_RI_BRCKT)) {
        raise(sc->line, "Expected ']' after array offset");
    }
    if(can_assign && match(ctx, sc, TK_ASSIGN)) {
        emit(ctx, OP_PUSH);
        parse_expr(ctx, sc, PREC_ASSIGNMENT);
        emit(ctx, OP_S_OFF); // 设置数组偏移
    } else {
        emit(ctx, OP_G_OFF); // 获取数组偏移
    }
}

// 解析三元条件表达式 (?:)
static void expr_ternary(ParseFunctionArgs) {
    emit(ctx, OP_JZ);
    uint64_t* addr_else = blank(ctx);
    parse_expr(ctx, sc, PREC_TERNARY);
    expect(ctx, sc, TK_COLON, "Expected ':' after '?' in ternary expression");
    emit(ctx, OP_JMP);
    uint64_t* addr_end = blank(ctx);
    patch(ctx, addr_else, ctx->btcode_cur - ctx->btcode);
    parse_expr(ctx, sc, PREC_TERNARY);
    patch(ctx, addr_end, ctx->btcode_cur - ctx->btcode);
}

// 解析列表初始化表达式 (e.g., {1, 2, 3})
static void expr_list(ParseFunctionArgs) {
    if(match(ctx, sc, TK_RI_BRACE)) {
        emit(ctx, OP_IMM);
        emit(ctx, 0); // 空列表
        return;
    }
    emit(ctx, OP_IMM);
    emit(ctx, (uint64_t)buildin_list); // 使用内置列表函数
    emit(ctx, OP_PUSH);
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
    ctx->expr_type = TYPE_INT + TYPE_PTR; // 列表类型为 int* (指针类型)
}

// 根据优先级解析表达式
static void parse_expr(context_t ctx, scanner sc, PrecLv level) {
    next(sc, ctx);
    ParseFn prefixFn = Rules[prev(sc).tk].prefix;
    if(prefixFn == NULL) {
        raise(sc->line, "Expected expression, but something else found");
    }
    int can_assign = level <= PREC_ASSIGNMENT; // 是否允许赋值
    prefixFn(ctx, sc, can_assign);
    while(level <= Rules[prst(sc).tk].prec) {
        next(sc, ctx);
        ParseFn infixFn = Rules[prev(sc).tk].infix;
        if(infixFn == NULL) {
            return;
        }
        infixFn(ctx, sc, can_assign);
    }
}

// 解析表达式语句
static void stmt_expr(ParseFunctionArgs) {
    parse_expr(ctx, sc, PREC_ASSIGNMENT); // 解析表达式
    expect(ctx, sc, TK_SEMICOLON, "Expected ';' after expression statement"); // 确保以分号结尾
}

// 解析代码块语句 ({ ... })
static void stmt_block(ParseFunctionArgs) {
    while(!match(ctx, sc, TK_RI_BRACE)) { // 解析代码块中的语句
        parse_stmt(ctx, sc);
    }
}

// 解析 return 语句
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

static void stmt_break(ParseFunctionArgs) {
    if(ctx->loop_depth == 0) {
        raise(sc->line, "Break statement not inside a loop");
    }
    emit(ctx, OP_JEND);
    emit(ctx, 0);
}

static void stmt_continue(ParseFunctionArgs) {
    if(ctx->loop_depth == 0) {
        raise(sc->line, "Continue statement not inside a loop");
    }
    emit(ctx, OP_JEND);
    emit(ctx, 1);
}

// 解析变量声明语句
static void stmt_decl(ParseFunctionArgs) {
    int base_type = ctx->expr_type;
    do{
        int real_type = base_type;
        token_t* id =  __identifier(ctx, sc, &real_type); // 解析标识符
        if(id->klass == TK_LOC) {
            raise(sc->line, "Variable '%.*s' already defined", id->len, id->name);
        } else if(id->klass == TK_GLO || id->klass == TK_FUN || id->klass == TK_SYS) {
            id = SymAdd(ctx, *id); // 如果是全局变量或函数，则添加到符号表
        }
        if(real_type == TYPE_VOID) {
            raise(sc->line, "Variable '%.*s' cannot be of type void", id->len, id->name);
        }
        id->type = real_type;
        id->klass = TK_LOC;
        id->val.uval = id - ctx->sym_loc;
        if(match(ctx, sc, TK_ASSIGN)) {
            parse_expr(ctx, sc, PREC_ASSIGNMENT);
        } else {
            emit(ctx, OP_IMM);
            emit(ctx, 0);
        }
        emit(ctx, OP_PUSH);
    }while(match(ctx, sc, TK_COMMA));
    expect(ctx, sc, TK_SEMICOLON, "Expected ';' after variable declaration"); // 确保以分号结尾
}

// 解析 while 循环语句
static void stmt_while(ParseFunctionArgs) {
    expect(ctx, sc, TK_LE_PAREN, "Expected '(' after 'while'");
    uint64_t* addr_start = ctx->btcode_cur;
    parse_expr(ctx, sc, PREC_ASSIGNMENT);
    expect(ctx, sc, TK_RI_PAREN, "Expected ')' after 'while' condition");
    emit(ctx, OP_JZ);
    uint64_t* addr_end = blank(ctx);
    SymStartLoop(ctx);
    parse_stmt(ctx, sc);
    SymEndLoop(ctx);
    emit(ctx, OP_JMP);
    emit(ctx, addr_start - ctx->btcode);
    patch(ctx, addr_end, ctx->btcode_cur - ctx->btcode);
    patch_loop_jumps(ctx, addr_start, ctx->btcode_cur);
}

static void stmt_dowhile(ParseFunctionArgs) {
    uint64_t* addr_start = ctx->btcode_cur;
    SymStartLoop(ctx);
    parse_stmt(ctx, sc);
    SymEndLoop(ctx);
    expect(ctx, sc, TK_WHILE, "Expected 'while' after 'do'");
    expect(ctx, sc, TK_LE_PAREN, "Expected '(' after 'while'");
    parse_expr(ctx, sc, PREC_ASSIGNMENT);
    expect(ctx, sc, TK_RI_PAREN, "Expected ')' after 'while' condition");
    emit(ctx, OP_NOT);
    emit(ctx, OP_JZ);
    emit(ctx, addr_start - ctx->btcode);
    patch_loop_jumps(ctx, addr_start, ctx->btcode_cur);
}

// 解析 for 循环语句
static void stmt_for(ParseFunctionArgs) {
    expect(ctx, sc, TK_LE_PAREN, "Expected '(' after 'for'");
    size_t old_sym_idx = ctx->sym_idx;
    if(match(ctx, sc, TK_SEMICOLON)) {
    } else if(match_type(ctx, sc)) {
        stmt_decl(ctx, sc, 1); // 解析 for 循环的初始化部分
    } else {
        stmt_expr(ctx, sc, 1); // 解析 for 循环的初始化表达式
    }
    int vars_declared = ctx->sym_idx - old_sym_idx;
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
    SymStartLoop(ctx);
    parse_stmt(ctx, sc);
    SymEndLoop(ctx);
    emit(ctx, OP_JMP);
    emit(ctx, addr_start - ctx->btcode); // 跳转到循环开始
    uint64_t* exit_addr = ctx->btcode_cur;
    if(addr_end) {
        patch(ctx, addr_end, exit_addr - ctx->btcode);
    }
    for(int i = 0; i < vars_declared; i++) {
        emit(ctx, OP_POP);
    }
    patch_loop_jumps(ctx, addr_start, exit_addr);
    ctx->sym_idx = old_sym_idx;
}

// 解析 if 语句
static void stmt_if(ParseFunctionArgs){
    expect(ctx, sc, TK_LE_PAREN, "Expected '(' after 'if'");
    parse_expr(ctx, sc, PREC_ASSIGNMENT);
    expect(ctx, sc, TK_RI_PAREN, "Expected ')' after 'if' condition");
    emit(ctx, OP_JZ);
    uint64_t* if_end = blank(ctx);
    parse_stmt(ctx, sc);
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

// 解析单个语句
static void parse_stmt(context_t ctx, scanner sc) {
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
    } else if(match_type(ctx, sc)) {
        stmt_decl(ctx, sc, 1);
    } else if(match(ctx, sc, TK_SEMICOLON)) {
        // 空语句，什么都不做
    } else if(match(ctx, sc, TK_DO)){
        stmt_dowhile(ctx, sc, 1);
    } else if(match(ctx, sc, TK_BREAK)){
        stmt_break(ctx, sc, 1);
    } else if(match(ctx, sc, TK_CONTINUE)){
        stmt_continue(ctx, sc, 1);
    } else {
        stmt_expr(ctx, sc, 1); // 解析表达式语句
    }
}

static void parse_enum(context_t ctx, scanner sc){
    expect(ctx, sc, TK_LE_BRACE, "Expected '{' after 'enum' declaration");
    int enum_value = 0;
    while(!match(ctx, sc, TK_RI_BRACE)){
        expect(ctx, sc, TK_ID, "Expected identifier in enum declaration");
        token_t* id = SymFind(ctx, prev(sc));
        if(id->klass != 0) {
            raise(sc->line, "Enum member '%.*s' already defined", id->len, id->name);
        }
        id->klass = TK_SYS;
        id->type = TYPE_INT;
        if(match(ctx, sc, TK_ASSIGN)) {
            expect(ctx, sc, TK_NUM, "Expected number after '=' in enum declaration");
            enum_value = prev(sc).val.ival;
        }
        id->val.ival = enum_value;
        enum_value++;
        if(!match(ctx, sc, TK_COMMA)) { // 如果不是逗号分隔, 必然是最后一个枚举成员
            expect(ctx, sc, TK_RI_BRACE, "Expected ',' or '}' in enum declaration");
            break;
        }
    }
    expect(ctx, sc, TK_SEMICOLON, "Expected ';' after enum declaration");
}

// 解析函数声明
static void parse_function(context_t ctx, scanner sc, token_t* name, int need_jit){
    emit(ctx, OP_JMP);
    uint64_t* addr = blank(ctx); // 留白，函数结束地址
    name->klass = TK_FUN;
    if(need_jit){
        name->val.pval = jitalloc();
    } else {
        name->val.uval = ctx->btcode_cur - ctx->btcode; // 函数地址为当前字节码位置
    }
    emit(ctx, OP_FUNC); // 函数入口标记
    SymSetloc(ctx);
    int arg_count = 0;
    while(!match(ctx, sc, TK_RI_PAREN)) { // 解析函数参数
        if(arg_count > 0) {
            expect(ctx, sc, TK_COMMA, "Expected ',' in function argument list");
        }
        if(!match_type(ctx, sc)) {
            raise(sc->line, "Expected type declaration for function argument");
        }
        int arg_type = ctx->expr_type; // 解析参数类型
        token_t* arg_id = __identifier(ctx, sc, &arg_type); // 解析参数标识符
        if(arg_id->klass == TK_LOC) {
            raise(sc->line, "Function argument '%.*s' already defined in this scope", arg_id->len, arg_id->name);
        } else if(arg_id->klass == TK_GLO || arg_id->klass == TK_FUN) {
            arg_id = SymAdd(ctx, *arg_id); // 如果是全局变量或函数，则添加到符号表
        }
        arg_id->type = arg_type; // 设置参数类型
        arg_id->klass = TK_LOC; // 设置为局部变量
        arg_id->val.uval = arg_id - ctx->sym_loc; // 计算局部变量的偏移量
        arg_count++;
    }
    expect(ctx, sc, TK_LE_BRACE, "Expected '{' after function declaration");
    stmt_block(ctx, sc, 1);
    SymEndloc(ctx);
    emit(ctx, OP_IMM);  // 配置默认返回值
    emit(ctx, 0);
    emit(ctx, OP_RET);
    patch(ctx, addr, ctx->btcode_cur - ctx->btcode); // 填充函数结束地址
    if(need_jit) {
        compile(ctx, name->val.pval, addr - ctx->btcode + 1, ctx->btcode_cur - ctx->btcode); // 编译静态函数
    }
}

// 解析定义变量时的赋值表达式
static void parse_assign(context_t ctx, scanner sc, token_t* id){
    parse_expr(ctx, sc, PREC_ASSIGNMENT);
    if(id->type != ctx->expr_type && (id->type == TYPE_FLOAT || ctx->expr_type == TYPE_FLOAT)) {
        raise(sc->line, "Cannot assign between float and int types directly");
    }
    int op_set_code = (ctx->sym_loc != 0) ? OP_S_LOC : OP_S_GLO;
    emit(ctx, op_set_code);
    emit(ctx, id->val.uval);
}


// 解析全局声明 (变量或函数)
static void parse_global(context_t ctx, scanner sc) {
    if(match(ctx, sc, TK_ENUM)) {
        parse_enum(ctx, sc); // 解析枚举声明
        return;
    }
    int in_static = match(ctx, sc, TK_STATIC); // 检查是否为静态变量，仅对函数生效。将函数jit编译为静态函数
    if(!match_type(ctx, sc)) {
        raise(sc->line, "Expected type declaration");
    }
    int base_type = ctx->expr_type; // 基本类型
    int real_type = ctx->expr_type; // 实际类型
    token_t* id = __identifier(ctx, sc, &real_type); // 解析标识符
    if(id->klass == TK_GLO || id->klass == TK_FUN) {
        raise(sc->line, "Global variable '%.*s' already defined", id->len, id->name);
    }
    id->type = real_type; // 设置变量类型
    if(match(ctx, sc, TK_LE_PAREN)){    // 函数声明
        parse_function(ctx, sc, id, in_static);
        return;
    }
    define_loop:
    id->klass = TK_GLO;
    id->val.uval = id - ctx->sym;
    if(match(ctx, sc, TK_ASSIGN)) {
        parse_expr(ctx, sc, PREC_ASSIGNMENT);
    } else {
        emit(ctx, OP_IMM); // 初始化为0
        emit(ctx, 0);
    }
    emit(ctx, OP_S_GLO);
    emit(ctx, id->val.uval);   // 使用全局变量的偏移量
    if(match(ctx, sc, TK_SEMICOLON)){
        return;
    } else if(!match(ctx, sc, TK_COMMA)) {
        raise(sc->line, "Expected ',' or ';' after global variable declaration");
    }
    real_type = base_type;
    id = __identifier(ctx, sc, &real_type); // 继续解析下一个标识符
    if(id->klass == TK_GLO || id->klass == TK_FUN) {
        raise(sc->line, "Global variable '%.*s' already defined", id->len, id->name);
    }
    id->type = real_type; // 设置变量类型
    goto define_loop;
}

// 编译源代码，生成字节码
void parse(context_t ctx, scanner sc) {
    next(sc, ctx); // 开始解析，扫描第一个token
    while(!match(ctx, sc, 0)) {
        parse_global(ctx, sc);// 全局声明
    }

    if(ctx->sym[ctx->main_id].klass != TK_FUN) {
        raise(sc->line, "Main function not defined");
    }

    emit(ctx, OP_IMM);
    emit(ctx, ctx->sym[ctx->main_id].val.uval);
    emit(ctx, OP_PUSH);
    emit(ctx, OP_PUSH);
    emit(ctx, OP_CALL); // 调用主函数
    emit(ctx, 0); // 主函数没有参数
    emit(ctx, OP_EXIT); // 程序结束指令
}
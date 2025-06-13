#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <stddef.h>

typedef enum {
    TK_NUM = 128, // 数字
    TK_FUN,       // 函数
    TK_SYS,       // 系统调用, build-in function
    TK_GLO,       // 全局变量
    TK_LOC,       // 局部变量
    TK_ID,        // 标识符
    TK_STR,       // 字符串
    TK_BREAK,     // break 关键字
    TK_CHAR,      // char 关键字
    TK_CONTINUE,  // continue 关键字
    TK_DO,        // do 关键字
    TK_ELSE,      // else 关键字
    TK_ENUM,      // enum 关键字
    TK_FOR,       // for 关键字
    TK_IF,        // if 关键字
    TK_INT,       // int 关键字
    TK_RETURN,    // return 关键字
    TK_SIZEOF,    // sizeof 关键字
    TK_WHILE,     // while 关键字
    TK_VOID,      // void 关键字
    TK_ASSIGN,    // 赋值操作符 '='
    TK_COND,      // 条件操作符 '?'
    TK_LOR,       // 逻辑或 '||'
    TK_LAN,       // 逻辑与 '&&'
    TK_NOT,       // 逻辑非 '!', new in C-lite, C4 is not supported
    TK_OR,        // 按位或 '|'
    TK_XOR,       // 按位异或 '^'
    TK_AND,       // 按位与 '&'
    TK_EQ,        // 相等操作符 '=='
    TK_NE,        // 不等操作符 '!='
    TK_LT,        // 小于操作符 '<'
    TK_GT,        // 大于操作符 '>'
    TK_LE,        // 小于等于操作符 '<='
    TK_GE,        // 大于等于操作符 '>='
    TK_SHL,       // 左移操作符 '<<'
    TK_SHR,       // 右移操作符 '>>'
    TK_ADD,       // 加法操作符 '+'
    TK_SUB,       // 减法操作符 '-'
    TK_MUL,       // 乘法操作符 '*'
    TK_DIV,       // 除法操作符 '/'
    TK_MOD,       // 取模操作符 '%'
    TK_INC,       // 自增操作符 '++'
    TK_DEC,       // 自减操作符 '--'
    TK_LE_PAREN, // 左括号 '('
    TK_RI_PAREN, // 右括号 ')'
    TK_LE_BRACE, // 左花括号 '{'
    TK_RI_BRACE, // 右花括号 '}'
    TK_LE_BRCKT, // 左方括号 '['
    TK_RI_BRCKT, // 右方括号 ']'
    TK_COMMA,     // 逗号 ','
    TK_SEMICOLON, // 分号 ';'
    TK_COLON,     // 冒号 ':'
} TkType;

typedef struct{
    TkType tk;     // token type
    TkType class;  // class of identifier (TK_GLO, TK_LOC, TK_FUN, TK_SYS)
    int type;      // type of identifier (Ctype)
    uint32_t hash; // hash value for identifier
    uint64_t val;  // 在编译时是符号表的偏移量，在运行时是变量的值
    char* name;    // pointer to name in symbol table, only valid for TK_ID and TK_STR
    size_t len;    // length of token name
} token_t;


#endif // __TOKEN_H__
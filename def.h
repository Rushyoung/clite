#ifndef __def_H__
#define __def_H__

#include "hash_table.h"
typedef enum{
    //keyword
    KEY_INT,         // int
    KEY_CHAR,        // char
    KEY_VOID,        // void
    KEY_FLOAT,       // float
    KEY_INTP,        // int *
    KEY_CHARP,       // char *
    KEY_FLOATP,      // float *
    KEY_IDENTIFIER,  // identifier
    KEY_IF,          // if
    KEY_ELSE,        // else
    KEY_WHILE,       // while
    KEY_RETURN,      // return

    //edge
    TK_LPAREN,      // (
    TK_RPAREN,      // )
    TK_LBRACE,      // {
    TK_RBRACE,      // }
    TK_LBRACKET,    // [
    TK_RBRACKET,    // ]
    TK_SEMICOLON,   // ;
    TK_COMMA,       // ,
    TK_PLUS,        // +
    TK_MINUS,       // -
    TK_MULTIPLY,    // *
    TK_DIVIDE,      // /
    TK_ASSIGN,      // =
    TK_EQ,          // ==
    TK_NEQ,         // !=
    TK_LT,          // <
    TK_GT,          // >
    TK_LE,          // <=
    TK_GE,          // >=
    TK_SHL,         // <<
    TK_SHR,         // >>
    TK_NOT,         // !
    TK_AND,         // &&
    TK_NUM_INT,     // 常整数
    TK_NUM_FLOAT,   // 常浮点数
    TK_STRING,      // 字符串常量
    TK_CHAR_LITERAL,//  字符常量
    TK_EOF          
} Tktype;



#endif
#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <stdint.h>

#define TOKEN(name, desc) name,
typedef enum {
    TK_START_FROM = 127,
    #include "token.def"
} TkType;
#undef TOKEN

typedef union __type {
    uint64_t    uval;
    int64_t     ival;
    double      fval;
    void       *pval;
} type_t;

typedef struct{
    TkType tk;     // token type
    TkType klass;  // class of identifier (TK_GLO, TK_LOC, TK_FUN, TK_SYS)
    int type;      // type of identifier (Ctype)
    uint32_t hash; // hash value for identifier
    type_t val;    // 在编译时是符号表的偏移量，在运行时是变量的值
    char* name;    // pointer to name in symbol table, only valid for TK_ID and TK_STR
    size_t len;    // length of token name
} token_t;


#endif // __TOKEN_H__
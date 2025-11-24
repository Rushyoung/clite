#ifndef __OPCODE_H__
#define __OPCODE_H__

#define OPCODE(name, desc) name,
typedef enum {
    OP_ZERO,
    #include "opcode.def"
} OpCode;
#undef OPCODE

enum {
    TYPE_VOID,   // 0 bytes, used for function return type
    TYPE_CHAR,   // 1 byte
    TYPE_INT,    // 8 bytes, which actually is long long in C
    TYPE_FLOAT,  // 8 bytes, double type
    TYPE_PTR,    // 8 bytes, pointer type
};


#endif // __OPCODE_H__
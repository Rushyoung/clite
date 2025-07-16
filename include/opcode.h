#ifndef __OPCODE_H__
#define __OPCODE_H__

typedef enum {
    OP_G_GLO=1, // 获取全局变量，一个操作数是偏移量
    OP_S_GLO,   // 设置全局变量，一个操作数是偏移量
    OP_G_LOC,   // 获取局部变量，一个操作数是偏移量
    OP_S_LOC,   // 设置局部变量，一个操作数是偏移量
    OP_IMM,     // 加载立即数，一个操作数
    OP_JMP,     // 无条件跳转，一个操作数是跳转地址
    OP_JZ,      // 如果为零则跳转, 一个操作数是跳转地址
    OP_JEND,    // 跳转到OP_LOOP处，一个操作数，0代表break，1代表continue
    OP_CALL,    // 调用函数，一个操作数是函数参数个数
    OP_RET,     // 返回函数
    OP_FUNC,    // 放在函数片段开头，仅代表这个地址是函数入口，无实际操作
    OP_LOOP,    // 放在循环末尾，仅代表这个地址是循环结束，无实际操作
    OP_PUSH,    // 压栈 = 12
    OP_G_OFF,   // 下标偏移量读
    OP_S_OFF,   // 下标偏移量写
    OP_OR,      // 按位或
    OP_XOR,     // 按位异或
    OP_AND,     // 按位与
    OP_EQU,     // 等于
    OP_LES,     // 小于
    OP_GRT,     // 大于
    OP_SHL,     // 左移
    OP_SHR,     // 右移
    OP_ADD,     // 加法 = 23
    OP_SUB,     // 减法
    OP_MUL,     // 乘法
    OP_DIV,     // 除法
    OP_MOD,     // 取模
    OP_NOT,     // 逻辑非
    OP_STR,     // 转为字符串
} OpCode;

enum {
    TP_VOID,   // 0 bytes, used for function return type
    TP_CHAR,   // 1 byte
    TP_INT,    // 8 bytes, which actually is long long in C
    TP_PTR,    // 8 bytes, pointer type
};


#endif // __OPCODE_H__
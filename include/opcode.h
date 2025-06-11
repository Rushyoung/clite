#ifndef __OPCODE_H__
#define __OPCODE_H__

typedef enum {
    OP_G_GLO=1, // 获取全局变量，一个操作数是偏移量
    OP_S_GLO,   // 设置全局变量，一个操作数是偏移量
    OP_G_LOC,   // 获取局部变量，一个操作数是偏移量
    OP_S_LOC,   // 设置局部变量，一个操作数是偏移量
    OP_IMM,     // 加载立即数，一个操作数
    OP_JMP,     // 无条件跳转，一个操作数是跳转地址
    OP_JZ,      // 如果为零则跳转
    OP_OFFSET,  // 下标偏移量，一个操作数是偏移量
    OP_CALL,    // 调用函数，一个操作数是函数参数个数
    OP_RET,     // 返回函数
    OP_FUNC,    // 放在函数片段开头，仅代表这个地址是函数入口，无实际操作
    OP_SAD,     // 保存地址，将当前栈地址保存到栈顶
    OP_PUSH,    // 压栈 = 12
    OP_OR,      // 按位或
    OP_XOR,     // 按位异或
    OP_AND,     // 按位与
    OP_EQU,     // 等于
    OP_LES,     // 小于
    OP_GRT,     // 大于
    OP_SHL,     // 左移
    OP_SHR,     // 右移
    OP_ADD,     // 加法 = 21
    OP_SUB,     // 减法
    OP_MUL,     // 乘法
    OP_DIV,     // 除法
    OP_MOD,     // 取模
    OP_NOT,     // 逻辑非
    OP_NEGATE,  // 逻辑负
    OP_STR,     // 转为字符串
    OP_OPEN,    // 打开文件 build-in = 29
    OP_READ,    // 读取文件 build-in
    OP_CLOSE,   // 关闭文件 build-in
    OP_PRINTF,  // 打印输出 build-in
    OP_INPUT,   // 输入 build-in，need free memory
    OP_MALLOC,  // 动态内存分配 build-in
    OP_FREE,    // 释放内存 build-in
    OP_MEMSET,  // 内存设置 build-in
    OP_MEMCMP,  // 内存比较 build-in
    OP_EXIT,    // 退出程序 build-in
} OpCode;

enum {
    TP_VOID,   // 0 bytes, used for function return type
    TP_CHAR,   // 1 byte
    TP_INT,    // 8 bytes, which actually is long long in C
    TP_PTR,    // 8 bytes, pointer type
};


#endif // __OPCODE_H__
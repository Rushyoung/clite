# Clite - A vm for a subset of C

Clite is a virtual machine that executes a subset of C code.

## subset of C
Clite is a subset of C that includes the following features:
- Basic data types: `int`, `char`, `void`, and their pointers, enums and arrays is mostly supported. 
- Basic operators: arithmetic operators (`+`, `-`, `*`, `/`, `%`), relational operators (`<`, `<=`, `>`, `>=`, `==`, `!=`), logical operators (`&&`, `||`, `!`), bitwise operators (`&`, `|`, `^`, `~`), and assignment operators (`=`).
- Basic variables: global variables, local variables, and function parameters.
- Control flow: `if`, `else`, `while`
- Functions: function definitions, function calls, and return values.
- built-in functions: `printf`, `open`, `malloc`, `free`, `exit`, `memset`, `memcpy`, `input`

Actually, the subset likes the [C4](https://github.com/rswier/c4) project.  
This project is rewritten from `C4` in a more easy-to-read way, and the code is more modularized.  
But sadly, the project is not self-compiled. QAQ  

## build the project
The project is written in C.  
So you can use any C compiler to build it, even the tinycc compiler.  
And we also provide the `make.bat` and `make.sh` scripts to build the project.
```shell
sh ./make/make.sh
# or
./make/make.bat
# or
./make/make-tcc.bat
```

## test the project
Do not finish this step.


## basic variant type

| 类型   | 描述     | 包含指针 |
|------|----------| -----|
| int  | 整数     | 是    |
| char | 字符     | 是    |
| void | 无类型   | 是    |
| enum | 全局匿名枚举（先不写）|



## function support
| 特性 | 描述         | 示例 (Clite 语法)          | 注意 |
| --- | --- | --- | --- |
| 函数定义 | 支持用户定义函数，包含返回类型、函数名、参数列表和函数体 | `int add(int a, int b) { return a + b; }` | 不检查任何属性，如参数类型、返回类型等，即使是返回类型是 void 也可以返回值 |
| 函数调用 | 支持调用已定义的函数和内建函数，传递参数并接收返回值 | `int sum = add(3, 5);` | 内建函数见下表 |
| 参数传递 | 在虚拟机的栈上传递 |  |  |
| 返回值 | 函数可以返回一个值，或不返回值 (void) | `return result;` / `return;` | `return;` 等价于 `return 0;`，因为懒得判断函数的返回类型了 |
| 递归调用 | 允许，因为局部变量在栈上被分配 |  | 栈的深度有限，递归过深可能导致栈溢出 |

#### symbol table
| 表名 | 用途 | 内容 | 其他 |
| ---- | ---- | ---- | ---- |
| 全局符号表 | 存储所有全局作用域内定义的标识符 | 函数名，全局变量，全局匿名枚举的名称及其值 | 因为懒得写栈，所以局部变量的符号表其实就跟在全局符号表后面，并且当解析完函数时会清空局部符号表 |
| 局部符号表 | 存储函数定义域内定义的标识符 | 函数参数，局部变量在栈上的位置 | 也是因为懒得写栈，所以函数内其实就一个局部变量域，而不是以`{}`做区分 |
| 堆 | 存储字符串常量 | 字符串常量 | 没什么用，想想办法取消掉也不是不行 |
| 字节码 | 存储编译后的字节码 | 每条指令的操作码和操作数 | 由编译器生成 |

#### keyword table
1. char  
2. else  
3. if  
4. int  
5. return  
6. void  
7. while  
8. enum
9. sizeof
10. for

## built-in functions
| 函数名 | 描述 | 参数 | 返回值 | 注意 |
| --- | --- | --- | --- | --- |
| printf | 打印格式化字符串 | `const char *format, ...` | `int` (打印的字符数) | 支持 `%d`, `%s`, `%c`, `%p` 等格式 |
| open | 打开文件 | `const char *filename, const char *mode` | `int` (文件描述符) | 支持 `r`, `w`, `a` 等模式 |
| malloc | 分配内存 | `size_t size` | `void *` (指向分配的内存) | 分配的内存未初始化 |
| free | 释放内存 | `void *ptr` | `void` | 只能释放通过 `malloc` 分配的内存 |
| exit | 退出程序 | `int status` | `void` | 直接终止程序 |
| memset | 内存设置 | `void *ptr, int value, size_t num` | `void *` (指向设置后的内存) | 将 `num` 字节的内存设置为 `value` |
| memcpy | 内存拷贝 | `void *dest, const void *src, size_t num` | `void *` (指向目标内存) | 将 `src` 的 `num` 字节拷贝到 `dest` |
| input | 获取用户输入 | `char *buffer` | `int` (实际读取的字符数) | 从标准输入读取一行字符串，最多读取 `size - 1` 个字符，最后添加 `\0` |
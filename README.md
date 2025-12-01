# Clite - A vm for a subset of C

Clite is a virtual machine that executes a subset of C code.

## subset of C
Clite is a subset of C that includes the following features:
- Basic data types: `int`, `char`, `void`, and their pointers, enums and arrays is mostly supported. 
- Basic operators: arithmetic operators (`+`, `-`, `*`, `/`, `%`), relational operators (`<`, `<=`, `>`, `>=`, `==`, `!=`), logical operators (`&&`, `||`, `!`), bitwise operators (`&`, `|`, `^`, `~`), and assignment operators (`=`).
- Basic variables: global variables, local variables, and function parameters.
- Control flow: `if`, `else`, `while`, `do-while`, `for`, `break`, `continue`, and `return`.
- Functions: function definitions, function calls, and return values.
- built-in functions: `printf`, `open`, `malloc`, `free`, `exit`, `memset`, `memcpy`, `fgets`, `rand`, `time`, `sleep`.

The subset likes the [C4](https://github.com/rswier/c4) project, but now is bigger than it.  
This project is rewritten from `C4` in a more easy-to-read way, and the code is more modularized.  
But sadly, the project is not self-compiled. QAQ  

## what is the meaning of the branch name in clite
branch | description
--- | ---
main | 由rushyoung创建，主分支，并未完成
beta | 由WuJunkai2004创建，完成了基本的语法分析和语义分析，支持了大部分C语言的特性，支持基于字节码的虚拟机执行，支持基于JIT的函数运行优化，支持数据类型。是一个稳定的版本
zeta | 由WuJunkai2004创建，支持从字节码转机器码的JIT编译，似乎可以支持全部的x86_64平台，但是只支持了部分字节码
theta | 由WuJunkai2004创建，因为theta符号有时用于Big O符号的变体，故该分支用于优化
iota | 由WuJunkai2004创建，在虚拟机和解析器上，支持数据类型

## build the project
The project is written in C.  
So you can use any C compiler to build it, even the tinycc compiler.  
And we provide `CMakeLists.txt` to build the project.  
The `zeta` branch is the JIT version of the project, which is not finished yet and noly works on Windows.  
Maybe it can be supported to Linux, but I don't to test it completionly.  
```shell
mkdir build
cd build
cmake ..
cmake --build . --config Release
```
or if you want to compile it with one command:
```shell
gcc -o clite.exe main.c .\src\*.c -Iinclude -Ofast
```

## test the project
```
cmake --build . --config Release --target test
```


## basic variant type

| 类型   | 描述     | 包含指针 |
|------|----------| -----|
| int  | 整数     | 是    |
| float | 浮点数   | 是    |
| char | 字符     | 是    |
| void | 无类型   | 是    |
| enum | 全局匿名枚举 |
| static | 静态函数，会进行jit编译 |



## function support
| 特性 | 描述         | 示例 (Clite 语法)          | 注意 |
| --- | --- | --- | --- |
| 函数定义 | 支持用户定义函数，包含返回类型、函数名、参数列表和函数体 | `int add(int a, int b) { return a + b; }` | 不检查参数类型，即使是返回类型是 void 也可以返回值 |
| 函数调用 | 支持调用已定义的函数和内建函数，传递参数并接收返回值 | `int sum = add(3, 5);` | 内建函数见下表 |
| 参数传递 | 在虚拟机的栈上传递 |  |  |
| 返回值 | 函数可以返回一个值，或不返回值 (void) | `return result;` / `return;` | `return;` 等价于 `return 0;`，因为懒得判断函数的返回类型了 |
| 递归调用 | 允许，因为局部变量在栈上被分配 |  | 栈的深度有限，递归过深可能导致栈溢出 |

#### symbol table
| 表名 | 用途 | 内容 | 其他 |
| ---- | ---- | ---- | ---- |
| 全局符号表 | 存储所有全局作用域内定义的标识符 | 函数名，全局变量，全局匿名枚举的名称及其值 | 因为懒得写栈，所以局部变量的符号表其实就跟在全局符号表后面，并且当解析完函数时会清空局部符号表 |
| 局部符号表 | 存储函数定义域内定义的标识符 | 函数参数，局部变量在栈上的位置 | 利用C语言的函数调用，有限地实现作用域，但是无法和外部变量重名 |
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
11. do
12. break
13. continue
14. static

## built-in functions
| 函数名 | 描述 | 参数 | 返回值 | 注意 |
| --- | --- | --- | --- | --- |
| printf | 打印格式化字符串 | `const char *format, ...` | `int` (打印的字符数) | 支持 `%d`, `%s`, `%c`, `%p` 等格式 |
| open | 打开文件 | `const char *filename, const char *mode` | `int` (文件描述符) | 支持 `r`, `w`, `a` 等模式 |
| malloc | 分配内存 | `size_t size` | `void *` (指向分配的内存) | 分配的内存未初始化 |
| free | 释放内存 | `void *ptr` | `void` | 只能释放通过 `malloc` 分配的内存 |
| exit | 退出程序 | `int status` | `void` | 直接终止程序 |
| memset | 内存设置 | `void *ptr, int value, size_t num` | `void *` (指向设置后的内存) | 将 `num` 字节的内存设置为 `value` |
| memcmp | 内存比较 | `const void *ptr1, const void *ptr2, size_t num` | `int` (比较结果) | 返回 `0` 如果相等，负值如果 `ptr1 < ptr2`，正值如果 `ptr1 > ptr2` |
| fgets | 获取用户输入 | `char *buffer, int size, int fd` | `buffer` | 从标准输入读取一行字符串，最多读取 `size - 1` 个字符，最后添加 `\n` |
| rand | 生成随机数 | 无参数 | `int` (随机数) | 返回一个介于 `0` 到 `RAND_MAX` 之间的随机整数 |
| time | 获取当前时间 | `int *tloc` | `int` (当前时间) | 返回自纪元以来的秒数 |
| sleep | 休眠指定秒数 | `int seconds` | `void` | 使程序休眠指定的秒数


## 已知bug
1. 在函数调用时，参数的类型和数量皆不检查
2. 函数的返回值类型不检查，void函数也可以返回值并被接收。需修复为void函数允许返回值，但是不允许被接收

## 已进行的优化
1. 计划进行窥孔优化-常量折叠
2. 计划进行窥孔优化-无效代码消除
3. 计划进行窥孔优化-尾调用优化
4. 已完成 Direct Threaded Code (DTC) 优化，提升虚拟机执行效率，默认开启，使用`-d -O`可以关闭
5. 计划进行 JIT 的栈顶缓存优化

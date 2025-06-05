# Clite - A vm for a subset of C

Clite is a virtual machine that executes a subset of C code.

## subset of C
Clite is a subset of C that includes the following features:
- Basic data types: `int`, `char`, `void`, and their pointers, enums and arrays is mostly supported. 
- Control flow: `if`, `else`, `while`
- Functions: function definitions, function calls, and return values，the aritis of functions is limited to 6
- built-in functions: `printf`, `open`, `malloc`, `free`, `exit`, `memset`, `memcpy`

Actually, the subset is the same as the [C4](https://github.com/rswier/c4) project.  
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
```

## test the project
Do not finish this step.


## basic variant type

| 类型   | 描述     |
|------|----------|
| int  | 整数     |
| char | 字符     |
| void | 无类型   |
| float| 浮点数   |
| enum | 全局匿名枚举（先不写）|
| int[]| 整型数组 |
| int* | 整型指针 |
| char[]|字符数组 |
| char*| 字符指针 |
| float*|浮点数指针|
| float[]|浮点数数组|

## const type

| 类型   | 描述     |
|--------|----------|
|int|32bit|
|float|32bit|
|字符常量|'a'|
|字符串常量|"asd" char[]|


## function surpport

| 特性 | 描述         | 示例 (Clite 语法)          |
|--------------|------------------------------------------|---------------------------|
| 函数定义     | 支持用户定义函数，包含返回类型、函数名、参数列表和函数体 | `int add(int a, int b) { return a + b; }` |
| 函数调用     | 支持调用已定义的函数，传递参数并接收返回值 | `int sum = add(3, 5);`      |
| 参数传递     | (说明参数传递方式，例如值传递)           |                           |
| 返回值       | 函数可以返回一个值，或不返回值 (void)    | `return result;` / `return;` |
| 递归调用     | 允许，因为局部变量在栈上被分配                 |                           |

#### symbol table
|表名|用途|内容|
|----|----------|-----|
|全局符号表|存储所有全局作用域内定义的标识符|函数名，全局变量，全局匿名枚举的名称及其值|
|函数作用域符号表（栈）|存储函数定义域内定义的标识符|函数参数，局部变量 (编译器根据此信息确定活动记录布局)|
|常量表|存储程序中的字面常量|整数常量，浮点数常量，字符常量，字符串常量等|


#### edge table
1. [
1. ]
1. (
1. )
1. {
1. }
1. '
1. "
1. ,
1. ;
1. \+
1. \-
1. \*
1. /
1. =
1. ==
1. !=
1. \<\<
1. \>\>
1. !
1. &&
1. <
1. \>
1. \<=
1. \>=

#### keyword table
1. int  
1. char 
1. void 
1. float
1. if
1. else
1. while
1. return


## Token
Tktype  
lexme  
location(wait)  
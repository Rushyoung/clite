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
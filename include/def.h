#ifndef __DEF_H__
#define __DEF_H__

#include <stdint.h>
#include "token.h"

struct _context_t{
    size_t   sym_idx;   // current index in symbol table
    size_t   main_id;   // main function identifier index in symbol table
    token_t* sym;       // symbol table, size is 8192, can be changed
    token_t* sym_loc;   // local symbol table, pointer to the current local symbol table

    char*   heap;       // heap memory for dynamic allocation, just for string
    size_t  heap_cur;   // current position in heap memory

    uint64_t*   btcode;     // bytecode for the program, not used in this version
    uint64_t*   btcode_cur; // current position in bytecode

    int loop_depth; // current loop depth, used for break/continue statements
    int expr_type;  // current expression type, used for type checking and change
};

typedef struct _context_t* context_t;

context_t InitContext();
void      FreeContext(context_t ctx);

void      emit(context_t ctx, uint64_t op);
uint64_t* blank(context_t ctx);                              // 留白
void      patch(context_t ctx, uint64_t* addr, uint64_t op); // 填充留白

void      patch_loop_jumps(context_t ctx, uint64_t* addr_start, uint64_t* addr_end);

token_t*  SymFind(context_t ctx, token_t tk);
token_t*  SymAdd(context_t ctx, token_t tk);
void      SymSetloc(context_t ctx);
void      SymEndloc(context_t ctx);

void      SymStartLoop(context_t ctx);
void      SymEndLoop(context_t ctx);

struct _args_t{
    int debug;
    int bytecode;
    int symboltable;
    int compile_only;
    int tokenization;
    char* inputs;
};

extern struct _args_t __args__;

void InitArgs(int argc, char* argv[]);

#endif//__DEF_H__
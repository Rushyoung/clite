#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "def.h"
#include "debug.h"
#include "opcode.h"

#include "parser.h"
#include "scanner.h"
#include "token.h"

#include "runner.h"


char* load(const char* file_name, size_t* file_size){
    FILE* file = fopen(file_name, "rb");
    if(!file){
        perror("Failed to open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = malloc(*file_size + 1);
    if(!buffer){
        perror("Failed to allocate memory for file");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, *file_size, file);
    buffer[*file_size] = '\0'; // null-terminate the string
    fclose(file);

    return buffer;
}


int main(){
    context_t ctx = InitContext();

    size_t file_size = 0;
    char*  file_name = ".\\hello.c";
    char*  file_code = load(file_name, &file_size);

    scanner sc = InitScanner(file_size, file_code);
    //DumpScanner(sc);
    compile(ctx, sc);
    printf("main function in %d\n", ctx->sym[ctx->main_id].val);
    DumpBtcode(ctx);
    run(ctx);
    return 0;
}

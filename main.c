#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "def.h"
#include "debug.h"
#include "parser.h"

#include "scanner.h"
#include "runner.h"


char* load(const char* file_name, size_t* file_size){
    FILE* file = fopen(file_name, "rb");
    if(!file){
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = malloc(*file_size + 1);
    if(!buffer){
        perror("Failed to allocate memory for file");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fread(buffer, 1, *file_size, file);
    buffer[*file_size] = '\0'; // null-terminate the string
    fclose(file);

    return buffer;
}


int main(int argc, char *argv[]){
    if(argc == 1){
        fprintf(stderr, "Usage: clite <source_file> [options]\n");
        return 1;
    }
    InitArgs(argc, argv);

    size_t file_size = 0;
    char*  file_code = load(__args__.inputs, &file_size);

    if(__args__.tokenization){
        DumpScanner(InitScanner(file_size, file_code));
    }

    context_t ctx = InitContext();
    scanner sc = InitScanner(file_size, file_code);
    parse(ctx, sc);
    if(__args__.symboltable){
        DumpSymbol(ctx);
    }
    if(__args__.bytecode){
        DumpBtcode(ctx);
    }
    if(!__args__.compile_only){
        run(ctx);
    }
    return 0;
}

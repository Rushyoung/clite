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


int main(int argc, char **argv){
    context_t ctx = InitContext();
	char* file_name;
    size_t file_size = 0;
    if(argc < 2){
        fprintf(stderr, "Usage: %s <source_file>\n", argv[0]);
        return 1;
    }
    else{
        file_name = argv[1];
    }

    char*  file_code = load(file_name, &file_size);

    scanner sc = InitScanner(file_size, file_code);
    compile(ctx, sc);
    DumpBtcode(ctx);
    run(ctx);
    return 0;
}

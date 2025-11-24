#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(){
    printf("clite version %s\n", __VERSION__);
    int fd = open("./example/file.c", 0);
    if (fd < 0)
        fd = open("./file.c", 0);
    if (fd < 0) {
        printf("Error opening file");
        return 1;
    }
    char *buffer = malloc(1024);
    if(buffer == NULL) {
        printf("Memory allocation failed");
        close(fd);
        return 1;
    }
    int bytesRead = read(fd, buffer, 1024);
    if (bytesRead < 0) {
        printf("Error reading file");
        free(buffer);
        close(fd);
        return 1;
    }
    printf("Read %d bytes from self.c:\n\n", bytesRead);
    printf("%.*s\n", bytesRead, buffer);
    free(buffer);
}
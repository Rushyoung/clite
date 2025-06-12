#include <stdio.h>
#include <unistd.h>

int main(){
    int start, end;
    printf("time = %d\n", start = time(NULL));
    printf("will sleep for 5 seconds...\n");
    sleep(5);
    printf("time = %d\n", end = time(NULL));
    printf("sleep finished\n");
    printf("elapsed time = %d seconds\n", end - start);
}
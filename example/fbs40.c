#include <stdio.h>

int fbs(int n){
    if(n <= 1) return n;
    return fbs(n - 1) + fbs(n - 2);
}

int main()
{
    for(int i = 1; i <= 40; i++){
        printf("fbs(%d) = %d\n", i, fbs(i));
    }
}
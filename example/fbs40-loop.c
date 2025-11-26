#include <stdio.h>

int fbs(int n){
    int a = 0, b = 1, c;
    if(n == 0) return a;
    for(int i = 2; i <= n; i++){
        c = a + b;
        a = b;
        b = c;
    }
    return b;
}

int main()
{
    for(int i = 1; i <= 40; i++){
        printf("fbs(%d) = %d\n", i, fbs(i));
    }
}
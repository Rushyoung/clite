#include <stdio.h>


int is_prime(int n) {
    if (n <= 1) return 0; // 0和1不是素数
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return 0; // 如果能被i整除，则不是素数
    }
    return 1; // 是素数
}


int number(char* str) {
    int n = 0;
    for(int i = 0; str[i]; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            n = n * 10 + (str[i] - '0'); // 将字符转换为数字
        } else {
            return -1; // 如果遇到非数字字符，返回-1
        }
    }
    return n;
}


int main(){
    printf("will check if the number and the next 10 numbers are prime.\n");
    printf("Enter a number: ");
    char* get = "                      ";
    input(get);
    int n = number(get);
    if (n == -1) {
        printf("Invalid input.\n");
        return 1;
    }
    for(int i = n; i <= n + 10; i++) {
        if (is_prime(i)) {
            printf("number %d is prime.\n", i);
        } else {
            printf("number %d is not prime.\n", i);
        }
    }
}
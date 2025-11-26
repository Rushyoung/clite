#include <stdio.h>
#include <winnt.h>


int main() {
    // 九九乘法表
    for(int i = 1; i <= 9; i++) {
        for(int j = 1; j <= i; j++) {
            printf("%dx%d=%-2d ", j, i, i * j);
        }
        printf("\n");
    }
    // 验证 continue 和 break
    for(int i = 0; i < 10; i++) {
        if(i % 2 == 0) {
            continue; // 跳过偶数
        }
        if(i > 7) {
            break; // 在7时跳出循环
        }
        printf("Odd number: %d\n", i);
    }
}
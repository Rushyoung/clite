#include <stdio.h>

// 简单的浮点加法函数
float add(float a, float b) {
    return a + b;
}

// 简单的浮点减法函数
float sub(float a, float b) {
    return a - b;
}

int main() {
    float a = 10.5;
    float b = 2.5;

    printf("=== Float Arithmetic Test ===\n");
    printf("a = %f, b = %f\n", a, b);

    // 加法
    float sum = add(a, b);
    printf("a + b = %f\n", sum);

    // 减法
    float diff = sub(a, b);
    printf("a - b = %f\n", diff);

    // 乘法
    float prod = a * b;
    printf("a * b = %f\n", prod);

    // 除法
    float quot = a / b;
    printf("a / b = %f\n", quot);

    printf("\n=== Float Comparison Test ===\n");
    float x = 3.14;
    float y = -2.0;
    
    // 大于比较
    if (x > y) {
        printf("%f is greater than %f\n", x, y);
    }

    // 相等比较 (注意：实际浮点比较通常不用==，这里仅测试逻辑)
    float z = 2.0;
    if (y == z) {
        printf("%f is equal to %f\n", y, z);
    }

    printf("\n=== Loop with Float ===\n");
    // 浮点数循环测试
    for (float i = 0.0; i < 5.0; i = i + 1.0) {
        printf("i = %f\n", i);
    }
    for(float j = 5.0; j > -5.0; j = j - 1.0) {
        printf("j = %f\n", j);
    }

    return 0;
}
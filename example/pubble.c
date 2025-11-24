#include <stdio.h>

int main() {
    int* arr = {64, 34, 25, 12, 22};  // 待排序数组
    int i;
    // 打印原始数组
    for (i = 0; i < 5; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    // 冒泡排序
    for (i = 0; i < 4; i++) {         // 外层循环控制轮数
        for (int j = 0; j < 4 - i; j++) { // 内层循环比较相邻元素
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }

    // 打印排序后数组
    for (i = 0; i < 5; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    return 0;
}
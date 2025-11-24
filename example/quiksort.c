#include <stdio.h>

// 交换数组中两个元素的值
void swap(int* arr, int i, int j) {
    int temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
}

// 分区函数
int partition(int* arr, int low, int high) {
    int pivot = arr[high];  // 选择最后一个元素作为基准
    int i = low - 1;        // 小于基准的元素的索引

    for (int j = low; j <= high - 1; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(arr, i, j);
        }
    }
    swap(arr, i + 1, high);
    return i + 1;
}

// 快速排序主函数
void quickSort(int* arr, int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);

        // 递归排序分区
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

// 测试代码
int main() {
    int* arr = malloc(12 * 8);
    int n = 12, i;


    for (i = 0; i < n; i++) {
        printf("%2d ", arr[i] = rand() % 100);
    }
    printf("\n");
    quickSort(arr, 0, n - 1);

    for (i = 0; i < n; i++) {
        printf("%2d ", arr[i]);
    }
    printf("\n");

    return 0;
}
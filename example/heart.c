#include <stdio.h>

int main() {
    char output;
    float x, y;
    float a, ans;
    for (y = 1.5; y > -1.5; y = y - 0.1) {
        for (x = -1.5; x < 1.5; x = x + 0.05) {
            a = x * x + y * y - 1.0;
            ans = a * a * a - x * x * y * y * y;
            if(ans <= 0.0){
                output = '*';
            } else {
                output = '.';
            }
            printf("%c", output);
        }
        printf("\n");
    }
}
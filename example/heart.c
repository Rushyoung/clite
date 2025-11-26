#include <stdio.h>

int main() {
    char output;
    for (float y = 1.5; y > -1.5; y = y - 0.1) {
        for (float x = -1.5; x < 1.5; x = x + 0.05) {
            float a = x * x + y * y - 1;
            output = (a * a * a - x * x * y * y * y <= 0.0 ? '*' : '.');
            printf("%c", output);
        }
        printf("\n");
    }
}
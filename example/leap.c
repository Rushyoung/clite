#include <inttypes.h>
#include <stdio.h>
int is_leap_year_fast(int y) {
    return ((y * 1073750999) & 3221352463)  <= 126976;
}
int main(){
    int res = is_leap_year_fast(2000);
    printf("%d", res);
}
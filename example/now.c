#include <stdio.h>
#include <time.h>

int is_leap_year(int year){
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}


int main(){
    int year, month, day, hour, minute, second;
    int now = time(NULL);

    for(year = 1970; now >= 365 * 24 * 3600; year++){
        if(is_leap_year(year)){
            now = now - (366 * 24 * 3600);
        } else {
            now = now - (365 * 24 * 3600);
        }
    }

    int *month_days = {31, 28+ is_leap_year(year), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    for(month = 0; now >= month_days[month] * 24 * 3600; month++){
        now = now - month_days[month] * 24 * 3600;
    }

    day = now / (24 * 3600);
    now = now % (24 * 3600);
    hour = now / 3600 + 8; // Adjusting for UTC+8 timezone
    now = now % 3600;
    minute = now / 60;
    second = now % 60;
    printf("Current time: ");
    printf("%d-%02d-%02d ", year, month + 1, day + 1);
    printf("%02d:%02d:%02d\n", hour, minute, second);
}
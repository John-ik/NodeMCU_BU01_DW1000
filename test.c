#include <stdio.h>
unsigned int a = 0x0A0B0C;

unsigned short data[5]; 

int main(){
    data[0] = a;

    printf("%d %d %d" ,data[0], data[1], data[3]);

    return 0;
}
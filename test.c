#include <stdio.h>
#include <stdint.h>
unsigned int a = 0x0A0B0C;

unsigned short data[5]; 

int main(){
    data[0] = a;

    char uart_buf[128] = {0,};
    uint16_t frame_len = 12;
    uint8_t rx_buffer[] = {0x41, 0x88, 0, 0xFF, 0xFF, 0xFF, 0xFF, 'V', 'E', 0x21, 0, 0};

    for (uint8_t i = 0; i < frame_len; i++){
        sprintf(uart_buf+2*i, "%02X", rx_buffer[i]);
    }
    printf("0x%s\n", uart_buf);

    return 0;
}
#pragma once

#ifdef DEBUG_UART_TRANSMIT
  #define DEBUG_PRINT_ENABLE

  #include "usart.h"
  #define Transmit(data_ptr) HAL_UART_Transmit(&huart1, (uint8_t*) data_ptr, strlen(data_ptr), 50)

#endif
#ifdef DEBUG_USB_TRANSMIT
  #define DEBUG_PRINT_ENABLE

  #include "usbd_cdc_if.h"
  #define Transmit(data_ptr) CDC_Transmit_FS((uint8_t*) data_ptr, strlen(data_ptr))

#endif


#ifdef DEBUG_PRINT_ENABLE

#include "string.h"
#include "stdio.h"
#include "stdint.h"
#include "deca_device_api.h"

extern char DEBUG_uart_buf[];
extern uint32 DEBUG_sys_ts;

// +68 вырезает начало путей до файла
// делает путь относительно папки проекта (зависит от папки)

#define DEBUG_transmit_fmt(fmt, ...) do{                                                                                                         \
    DEBUG_sys_ts = dwt_readsystimestamphi32();                                                                                                                 \
    sprintf(DEBUG_uart_buf, "%s:%d:0x%08lX00: " fmt "\n", __FILE__, __LINE__, DEBUG_sys_ts, __VA_ARGS__); \
    Transmit(DEBUG_uart_buf);                                                                                                                    \
  }while(0)

#define DEBUG_transmit_str(str) DEBUG_transmit_fmt("%s", str)

#define DEBUG_transmit_b10(str, bytes) DEBUG_transmit_fmt(str " = 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", bytes[0],bytes[1],bytes[2],bytes[3],bytes[4],bytes[5],bytes[6],bytes[7],bytes[8],bytes[9])

#else

#define DEBUG_transmit_fmt(fmt, ...) __NOP()
#define DEBUG_transmit_str(str) __NOP()
#define DEBUG_transmit_b10(str, bytes) __NOP()

#endif

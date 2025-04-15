#pragma once

#include "usart.h"

#include "string.h"
#include "stdio.h"
#include "stdint.h"




#ifdef DEBUG
  static uint8_t DEBUG_uart_buf[512];

#define DEBUG_transmit_fmt(fmt, ...) \
  sprintf((char*)DEBUG_uart_buf, "%s:%d: " fmt "\n", __FILE__, __LINE__, __VA_ARGS__); HAL_UART_Transmit(&huart1, DEBUG_uart_buf, strlen((char*) DEBUG_uart_buf), 100)
  
#define DEBUG_transmit_str(str) DEBUG_transmit_fmt("%s", str)
#endif

#define DEBUG_transmit_b10(str, bytes) DEBUG_transmit_fmt(str " = 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", bytes[0],bytes[1],bytes[2],bytes[3],bytes[4],bytes[5],bytes[6],bytes[7],bytes[8],bytes[9])
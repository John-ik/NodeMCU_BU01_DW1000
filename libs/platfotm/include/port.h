#pragma once

#include "spi.h"
#include "stdint.h"

#include "deca_device_api.h"

// -------------------- CONFIG --------------------

#define SPIx SPI1

#define DW_SPI_TIMEOUT 1000

#define SPIx_CS_GPIO  CSN_GPIO_Port
#define SPIx_CS   CSN_Pin

#define DW_IRQ_Port IRQ_GPIO_Port
#define DW_IRQ_Pin  IRQ_Pin
#define DW_EXTI_IRQ      EXTI0_IRQn

#define DW1000_RSTn_GPIO RST_GPIO_Port
#define DW1000_RSTn  RST_Pin

// -------------------- CONSTS --------------------
#define DECA_MAX_SPI_HEADER_LENGTH      (3)                     // max number of bytes in header (for formating & sizing)

// -------------------- PLATFORM SPECIFIC API --------------------

ITStatus EXTI_GetITEnStatus(uint32_t EXTI_Line);


void led_signal (uint8_t signal);

void reset_DW1000(void);
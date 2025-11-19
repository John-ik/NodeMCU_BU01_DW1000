#pragma once

#include "spi.h"
#include "stdint.h"

#include "deca_device_api.h"

// -------------------- CONFIG --------------------

#define DW_SPI hspi1

#define DW_SPI_TIMEOUT 1000

#define DW_CS_Port  CSN_GPIO_Port
#define DW_CS_Pin   CSN_Pin

#define DW_IRQ_Port IRQ_GPIO_Port
#define DW_IRQ_Pin  IRQ_Pin
#define DW_EXTI_IRQ      EXTI0_IRQn

#define DW_RST_Port RST_GPIO_Port
#define DW_RST_Pin  RST_Pin

// -------------------- CONSTS --------------------

#define NUM_LEDS 3

// -------------------- PLATFORM SPECIFIC API --------------------

ITStatus EXTI_GetITEnStatus(uint32_t EXTI_Line);


void led_signal (uint8_t signal);

void reset_DW1000(void);

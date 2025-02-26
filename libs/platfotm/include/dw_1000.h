#pragma once

#include "spi.h"
#include "stdint.h"


// -------------------- CONFIG --------------------

#define DW_SPI hspi1

#define DW_SPI_TIMEOUT 1000

#define DW_CS_Port CSN_GPIO_Port
#define DW_CS_Pin  CSN_Pin


// -------------------- SPI --------------------

void DW_spi_write (uint16_t headerLength, const uint8_t *headerBuffer, uint32_t bodylength, const uint8_t *bodyBuffer);

void DW_spi_read (uint16_t headerLength, const uint8_t *headerBuffer, uint32_t bodylength, uint8_t *bodyBuffer);
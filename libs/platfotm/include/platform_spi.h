#pragma once

#include "main.h"
#include "stm32f1xx_hal_spi.h"


#define MHZ 1000000

int spi_low_speed();

int spi_full_speed();

// Defines a default timeout delay in milliseconds for the SPI transfer
#ifndef SPI_TRANSFER_TIMEOUT
    #define SPI_TRANSFER_TIMEOUT 1000
#elif SPI_TRANSFER_TIMEOUT <= 0
    #error "SPI_TRANSFER_TIMEOUT cannot be less or equal to 0!"
#endif

///@brief SPI errors
typedef enum {
    SPI_OK = 0,
    SPI_TIMEOUT = 1,
    SPI_ERROR = 2
} spi_status_e;

#define JUNK 0x0

#define SPI_TRANSMITRECEIVE false
#define SPI_TRANSMITONLY true

spi_status_e spi_transfer(SPI_TypeDef *obj, const uint8_t *tx_buffer, uint8_t *rx_buffer, uint16_t len);
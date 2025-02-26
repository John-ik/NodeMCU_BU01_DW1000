#include "dw_1000.h"
#include "main.h"

#include "string.h"

#include "debug.h"


static uint8_t spi_buf_tx[1024]; 
static uint8_t spi_buf_rx[1024]; 


void dw_activate(){
  HAL_GPIO_WritePin(DW_CS_Port, DW_CS_Pin, 0);
}

void dw_deactivate(){
  HAL_GPIO_WritePin(DW_CS_Port, DW_CS_Pin, 1);
}

void DW_spi_write (
    uint16_t headerLength, const uint8_t *headerBuffer, 
    uint32_t bodylength, const uint8_t *bodyBuffer
){
  dw_activate();
  
  memcpy(spi_buf_tx, headerBuffer, headerLength);
  memcpy(spi_buf_tx + headerLength, bodyBuffer, bodylength);

  size_t buf_length = headerLength + bodylength; 

  HAL_SPI_Transmit(&DW_SPI, spi_buf_tx, buf_length, DW_SPI_TIMEOUT);

  dw_deactivate();
}

void DW_spi_read (
  uint16_t headerLength, const uint8_t *headerBuffer, 
  uint32_t bodylength, uint8_t *bodyBuffer
){
  dw_activate();
  
  memcpy(spi_buf_tx, headerBuffer, headerLength);
  memset(spi_buf_rx, 0, sizeof(spi_buf_rx));


  HAL_SPI_TransmitReceive(&DW_SPI, spi_buf_tx, spi_buf_rx, headerLength + bodylength, DW_SPI_TIMEOUT);
 
  memcpy(bodyBuffer, spi_buf_rx + headerLength, bodylength);

  dw_deactivate();
}
#include "port.h"
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

int writetospi (
    uint16 headerLength, const uint8 *headerBuffer, 
    uint32 bodylength, const uint8 *bodyBuffer
){
  dw_activate();
  
  memcpy(spi_buf_tx, headerBuffer, headerLength);
  memcpy(spi_buf_tx + headerLength, bodyBuffer, bodylength);

  size_t buf_length = headerLength + bodylength; 

  if (HAL_SPI_Transmit(&DW_SPI, spi_buf_tx, buf_length, DW_SPI_TIMEOUT) != HAL_OK)
    return DWT_ERROR;

  dw_deactivate();

  return DWT_SUCCESS;
}

int readfromspi (
  uint16 headerLength, const uint8 *headerBuffer, 
  uint32 bodylength, uint8 *bodyBuffer
){
  dw_activate();
  
  memcpy(spi_buf_tx, headerBuffer, headerLength);
  memset(spi_buf_rx, 0, sizeof(spi_buf_rx));


  if (HAL_SPI_TransmitReceive(&DW_SPI, spi_buf_tx, spi_buf_rx, headerLength + bodylength, DW_SPI_TIMEOUT) != HAL_OK)
    return DWT_ERROR;
 
  memcpy(bodyBuffer, spi_buf_rx + headerLength, bodylength);

  dw_deactivate();

  return DWT_SUCCESS;
}
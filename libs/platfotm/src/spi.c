#include "port.h"
#include "platform_spi.h"

#include "main.h"

#include "string.h"

#include "debug.h"


void dw_activate(){
  HAL_GPIO_WritePin(DW_CS_Port, DW_CS_Pin, 0);
}

void dw_deactivate(){
  HAL_GPIO_WritePin(DW_CS_Port, DW_CS_Pin, 1);
}


/**
  * @brief This function is implemented by user to send/receive data over
  *         SPI interface
  * @param  spi : pointer to SPI_TypeDef structure
  * @param  tx_buffer : tx data to send before reception
  * @param  rx_buffer : rx data to receive if not numm
  * @param  len : length in byte of the data to send and receive
  * @retval status of the send operation (0) in case of error
  */
spi_status_e spi_transfer(SPI_TypeDef *spi, const uint8_t *tx_buffer, uint8_t *rx_buffer, uint16_t len)
{
  spi_status_e ret = SPI_OK;
  uint32_t tickstart, size = len;
  SPI_TypeDef *_SPI = spi;
  uint8_t *tx_buf = (uint8_t *)tx_buffer;

  if (len == 0) {
    ret = SPI_ERROR;
  } else {
    tickstart = HAL_GetTick();

    #if defined(SPI_CR2_TSIZE)
    /* Start transfer */
    LL_SPI_SetTransferSize(_SPI, size);
    LL_SPI_Enable(_SPI);
    LL_SPI_StartMasterTransfer(_SPI);
    #endif

    while (size--) {
      #if defined(SPI_SR_TXP)
      while (!LL_SPI_IsActiveFlag_TXP(_SPI));
      #else
      while (!LL_SPI_IsActiveFlag_TXE(_SPI));
      #endif
      LL_SPI_TransmitData8(_SPI, tx_buf ? *tx_buf++ : 0XFF);

      #if defined(SPI_SR_RXP)
      while (!LL_SPI_IsActiveFlag_RXP(_SPI));
      #else
      while (!LL_SPI_IsActiveFlag_RXNE(_SPI));
      #endif
      if (rx_buffer) {
        *rx_buffer++ = LL_SPI_ReceiveData8(_SPI);
      } else {
        LL_SPI_ReceiveData8(_SPI);
      }
      if ((SPI_TRANSFER_TIMEOUT != HAL_MAX_DELAY) && (HAL_GetTick() - tickstart >= SPI_TRANSFER_TIMEOUT)) {
        ret = SPI_TIMEOUT;
        break;
      }
    }

    #if defined(SPI_IFCR_EOTC)
    // Add a delay before disabling SPI otherwise last-bit/last-clock may be truncated
    // See https://github.com/stm32duino/Arduino_Core_STM32/issues/1294
    // Computed delay is half SPI clock
    delayMicroseconds(obj->disable_delay);

    /* Close transfer */
    /* Clear flags */
    LL_SPI_ClearFlag_EOT(_SPI);
    LL_SPI_ClearFlag_TXTF(_SPI);
    /* Disable SPI peripheral */
    LL_SPI_Disable(_SPI);
    #else
    /* Wait for end of transfer */
    while (LL_SPI_IsActiveFlag_BSY(_SPI));
    #endif
  }
  return ret;
}

uint8_t spi_transfer_trx(uint8_t data){
  DEBUG_transmit_fmt("write 0x%X", data);
  spi_status_e e = spi_transfer(SPI1, &data, &data, sizeof(uint8_t));
  if (e) {DEBUG_transmit_fmt("e: %d", e);}
  DEBUG_transmit_fmt("read 0x%X", data);
  return data;
}


int writetospi (
    uint16 headerLength, const uint8 *headerBuffer, 
    uint32 bodylength, const uint8 *bodyBuffer
){
  uint16_t i;

  dw_activate();

  for(i = 0; i < headerLength; i++) {
		spi_transfer_trx(headerBuffer[i]); // send header
	}
	for(i = 0; i < bodylength; i++) {
		spi_transfer_trx(bodyBuffer[i]); // write values
	}

  dw_deactivate();

  return DWT_SUCCESS;
}

int readfromspi (
  uint16 headerLength, const uint8 *headerBuffer, 
  uint32 bodylength, uint8 *bodyBuffer
){
  uint16_t i;

  dw_activate();
  for(i = 0; i < headerLength; i++) {
		spi_transfer_trx(headerBuffer[i]); // send header
	}
	for(i = 0; i < bodylength; i++) {
		bodyBuffer[i] = spi_transfer_trx(JUNK); // read values
	}

  dw_deactivate();

  return DWT_SUCCESS;
}


int spi_set_BaudRatePrescaler(uint32_t prescaler){
  assert_param(IS_SPI_BAUDRATE_PRESCALER(prescaler));


  SPI1->CR1 = (SPI1->CR1 & ~SPI_CR1_BR_Msk) | (prescaler & SPI_CR1_BR_Msk);

  return HAL_OK;
}


int spi_low_speed(){
  return spi_set_BaudRatePrescaler(SPI_BAUDRATEPRESCALER_32);
}

int spi_full_speed(){
  return spi_set_BaudRatePrescaler(SPI_BAUDRATEPRESCALER_4);
}
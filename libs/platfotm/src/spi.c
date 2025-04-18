#include "port.h"
#include "main.h"

#include "string.h"

#include "debug.h"


/*! ------------------------------------------------------------------------------------------------------------------
 * Function: writetospi()
 *
 * Low level abstract function to write to the SPI
 * Takes two separate byte buffers for write header and write data
 * returns 0 for success, or -1 for error
 */
// #pragma GCC optimize ("O3")
int writetospi(uint16 headerLength, const uint8 *headerBuffer, uint32 bodylength, const uint8 *bodyBuffer)
{
    DEBUG_transmit_str("write_in");
	  uint32 i=0;

    decaIrqStatus_t  stat ;

    stat = decamutexon() ;

    SPIx_CS_GPIO->BRR = SPIx_CS;

    for(i=0; i<headerLength; i++)
    {
    	SPIx->DR = headerBuffer[i];

    	while ((SPIx->SR & SPI_FLAG_RXNE) == (uint16_t)RESET);

    	SPIx->DR ;
    }

    for(i=0; i<bodylength; i++)
    {
     	SPIx->DR = bodyBuffer[i];

    	while((SPIx->SR & SPI_FLAG_RXNE) == (uint16_t)RESET)
      {}

		SPIx->DR ;
	}

    SPIx_CS_GPIO->BSRR = SPIx_CS;

    decamutexoff(stat) ;
    DEBUG_transmit_str("write_out");
    return 0;
} // end writetospi()


/*! ------------------------------------------------------------------------------------------------------------------
 * Function: readfromspi()
 *
 * Low level abstract function to read from the SPI
 * Takes two separate byte buffers for write header and read data
 * returns the offset into read buffer where first byte of read data may be found,
 * or returns -1 if there was an error
 */
// #pragma GCC optimize ("O3")
int readfromspi(uint16 headerLength, const uint8 *headerBuffer, uint32 readlength, uint8 *readBuffer)
{
    DEBUG_transmit_str("read_in");
	  uint32 i=0;

    decaIrqStatus_t  stat ;

    stat = decamutexon() ;

    /* Wait for SPIx Tx buffer empty */
    //while (port_SPIx_busy_sending());

    SPIx_CS_GPIO->BRR = SPIx_CS;

    for(i=0; i<headerLength; i++)
    {
    	SPIx->DR = headerBuffer[i];

     	while((SPIx->SR & SPI_FLAG_RXNE) == (uint16_t)RESET);

     	readBuffer[0] = SPIx->DR ; // Dummy read as we write the header
    }

    for(i=0; i<readlength; i++)
    {
    	SPIx->DR = 0;  // Dummy write as we read the message body

    	while((SPIx->SR & SPI_FLAG_RXNE) == (uint16_t)RESET)
      {}
 
	   	readBuffer[i] = SPIx->DR ;//port_SPIx_receive_data(); //this clears RXNE bit
    }

    SPIx_CS_GPIO->BSRR = SPIx_CS;

    decamutexoff(stat) ;
    DEBUG_transmit_str("read_out");
    return 0;
} // end readfromspi()

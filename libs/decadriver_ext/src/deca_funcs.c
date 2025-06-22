#include "deca_funcs.h"

#include "deca_sleep.h"


dwt_irq_func_t _dwt_handler_cplock = NULL;
dwt_irq_func_t _dwt_handler_esyncr = NULL;
dwt_irq_func_t _dwt_handler_aat = NULL;
dwt_irq_func_t _dwt_handler_send = NULL;
dwt_irq_func_t _dwt_handler_rxfailed = NULL;
dwt_irq_func_t _dwt_handler_rxok = NULL;
dwt_irq_func_t _dwt_handler_rxtimeout = NULL;
dwt_irq_func_t _dwt_handler_rxoverrun = NULL;
dwt_irq_func_t _dwt_handler_sleep2init = NULL;
dwt_irq_func_t _dwt_handler_pll_error = NULL;
dwt_irq_func_t _dwt_handler_hpdwarn = NULL;


void dwt_custom_softReset() {
	uint8_t pmscctrl0[PMSC_CTRL0_LEN];
	dwt_readfromdevice(PMSC_ID, PMSC_CTRL0_OFFSET, PMSC_CTRL0_LEN, pmscctrl0);
	pmscctrl0[0] = 0x01;
	dwt_writetodevice(PMSC_ID, PMSC_CTRL0_OFFSET, PMSC_CTRL0_LEN, pmscctrl0);
	pmscctrl0[3] = 0x00;
	dwt_writetodevice(PMSC_ID, PMSC_CTRL0_OFFSET, PMSC_CTRL0_LEN, pmscctrl0);
	deca_sleep(10);
	pmscctrl0[0] = 0x00;
	pmscctrl0[3] = 0xF0;
	dwt_writetodevice(PMSC_ID, PMSC_CTRL0_OFFSET, PMSC_CTRL0_LEN, pmscctrl0);
  deca_sleep(10);
}

/**
 * @brief softreset receiver-only
 * @details reg 0x36:00 - 7.2.50.1
 */
void softreset_receiver(){
  uint8 buf = 0;

  dwt_readfromdevice(PMSC_ID, 0, 1, &buf);
  buf = (buf & 0x03) | 0x01; // set SYSCLKS to 01
  dwt_writetodevice(PMSC_ID, 0, 1, &buf);

  dwt_readfromdevice(PMSC_ID, 0x3, 1, &buf);
  buf &= 0xEF; // clear only bit 28 to reset only receiver
  dwt_writetodevice(PMSC_ID, 0x3, 1, &buf); 

  buf &= 0x1F; // set only bit 28 to reset only receiver
  dwt_writetodevice(PMSC_ID, 0x3, 1, &buf); 
}

uint64_t get_sys_ts(){
    return (uint64_t) (dwt_readsystimestamphi32() >> 8); // 9 lowers bits always zero -> USER MANUAL 7.2.8
}

uint64_t get_rx_ts(){
    return (uint64_t) (dwt_readrxtimestamphi32() >> 8) | (dwt_readrxtimestamplo32() & 0xff);
}

uint64_t get_tx_ts(){
    return (uint64_t) (dwt_readtxtimestamphi32() >> 8) | (dwt_readtxtimestamplo32() & 0xff);
}

uint32 dwt_get_status(){
    return dwt_read32bitreg(SYS_STATUS_ID);
}
void dwt_reset_status(uint32 status){
    dwt_write32bitreg(SYS_STATUS_ID, status & SYS_STATUS_MASK_32);
}

void dwt_irq(){
    uint32 status = dwt_get_status();
    uint32 status2reset = 0;
    
    if(status & DWT_IRQ_CPLOCK){
        __DWT_IRQ_CALL_HANDLER(_dwt_handler_cplock, status);
        status2reset |= DWT_IRQ_CPLOCK;
    }
    if(status & DWT_IRQ_ESYNCR){
        __DWT_IRQ_CALL_HANDLER(_dwt_handler_cplock, status);
        status2reset |= DWT_IRQ_ESYNCR;
    }
    if(status & DWT_IRQ_AAT){
        __DWT_IRQ_CALL_HANDLER(_dwt_handler_aat, status);
        status2reset |= DWT_IRQ_AAT;
    }
    if(status & DWT_IRQ_SEND){
        __DWT_IRQ_CALL_HANDLER(_dwt_handler_send, status);
        status2reset |= SYS_STATUS_ALL_TX ^ SYS_STATUS_AAT;
    }
    if(status & DWT_IRQ_RXFAILED){
        __DWT_IRQ_CALL_HANDLER(_dwt_handler_rxfailed, status);
        status2reset |= DWT_IRQ_RXFAILED;
    }
    if(status & DWT_IRQ_RXOK){
        __DWT_IRQ_CALL_HANDLER(_dwt_handler_rxok, status);
        status2reset |= SYS_STATUS_ALL_RX_GOOD;
    }
    if(status & DWT_IRQ_RXTIMEOUT){
        __DWT_IRQ_CALL_HANDLER(_dwt_handler_rxtimeout, status);
        status2reset |= DWT_IRQ_RXTIMEOUT;
    }
    if(status & DWT_IRQ_RXOVERRRUN){
        __DWT_IRQ_CALL_HANDLER(_dwt_handler_rxoverrun, status);
        status2reset |= DWT_IRQ_RXOVERRRUN;
    }
    if(status & DWT_IRQ_SLEEP2INIT){
        __DWT_IRQ_CALL_HANDLER(_dwt_handler_sleep2init, status);
        status2reset |= DWT_IRQ_SLEEP2INIT;
    }
    if(status & DWT_IRQ_PLL_ERROR){
        __DWT_IRQ_CALL_HANDLER(_dwt_handler_pll_error, status);
        status2reset |= DWT_IRQ_PLL_ERROR;
    }
    if(status & DWT_IRQ_HPDWARN){
        __DWT_IRQ_CALL_HANDLER(_dwt_handler_hpdwarn, status);
        status2reset |= DWT_IRQ_HPDWARN;
    }
    dwt_reset_status(status2reset);
}

void dwt_showDiag(char* buf){
    dwt_deviceentcnts_t cntrs;
    dwt_readeventcounters(&cntrs);
    sprintf(buf,
        "DIAG: "
        "PHR er = %u, "
        "RSD er = %u, "
        "crc good = %u, "
        "crc bad = %u, "
        "frame filter reject = %u, "
        "overrun = %u, "
        "sfd timeout = %u, "
        "preamble timeout = %u, "
        "RX timeout = %u, "
        "TX sent = %u, "
        "big delay = %u, "
        "short delay = %u",
    cntrs.PHE, cntrs.RSL, cntrs.CRCG, cntrs.CRCB, cntrs.ARFE, cntrs.OVER, cntrs.SFDTO, cntrs.PTO, cntrs.RTO, cntrs.TXF, cntrs.HPW, cntrs.TXW
    );
}

// -------------------- DEPRECATED --------------------


void configureTXPower(dwt_txconfig_t *config){
    config->PGdly = TX_PGDELAY_CH5;
    config->power = 0x1F1F1F1F;
    dwt_configuretxrf(config);
}

/**
 * @brief get systime in 64-bit 
 *        dwt return systime 40-bit value in 5 bytes
 * @return 40-bit systime in 64-bit
 */
uint64 get_systime_u64(void){
  uint8 buf[10];
  uint64 time = 0;
  dwt_readsystime(buf);
  for (int8 i = 4; i >= 0; i--){
    time <<= 8;
    time |= buf[i];
  }
  return time;
}


/*! ------------------------------------------------------------------------------------------------------------------
 * @fn get_tx_timestamp_u64()
 *
 * @brief Get the TX time-stamp in a 64-bit variable.
 *        /!\ This function assumes that length of time-stamps is 40 bits, for both TX and tx!
 *
 * @param  none
 *
 * @return  64-bit value of the read time-stamp.
 */
uint64 get_tx_timestamp_u64(void)
{
    uint8 ts_tab[5];
    uint64 ts = 0;
    int i;
    dwt_readtxtimestamp(ts_tab);
    for (i = 4; i >= 0; i--)
    {
        ts <<= 8;
        ts |= ts_tab[i];
    }
    return ts;
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn get_rx_timestamp_u64()
 *
 * @brief Get the RX time-stamp in a 64-bit variable.
 *        /!\ This function assumes that length of time-stamps is 40 bits, for both TX and RX!
 *
 * @param  none
 *
 * @return  64-bit value of the read time-stamp.
 */
uint64 get_rx_timestamp_u64(void)
{
    uint8 ts_tab[5];
    uint64 ts = 0;
    int i;
    dwt_readrxtimestamp(ts_tab);
    for (i = 4; i >= 0; i--)
    {
        ts <<= 8;
        ts |= ts_tab[i];
    }
    return ts;
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn final_msg_get_ts()
 *
 * @brief Read a given timestamp value from the final message. In the timestamp fields of the final message, the least
 *        significant byte is at the lower address.
 *
 * @param  ts_field  pointer on the first byte of the timestamp field to read
 *         ts  timestamp value
 *
 * @return none
 */
void final_msg_get_ts(const uint8 *ts_field, uint32 *ts)
{
    int i;
    *ts = 0;
    for (i = 0; i < FINAL_MSG_TS_LEN; i++)
    {
        *ts += ts_field[i] << (i * 8);
    }
}
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn final_msg_set_ts()
 *
 * @brief Fill a given timestamp field in the final message with the given value. In the timestamp fields of the final
 *        message, the least significant byte is at the lower address.
 *
 * @param  ts_field  pointer on the first byte of the timestamp field to fill
 *         ts  timestamp value
 *
 * @return none
 */
void final_msg_set_ts(uint8 *ts_field, uint64 ts)
{
    int i;
    for (i = 0; i < FINAL_MSG_TS_LEN; i++)
    {
        ts_field[i] = (uint8) ts;
        ts >>= 8;
    }
}

#pragma once

#include "stdint.h"

#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_types.h"

#include "messages.h"


void dwt_custom_softReset();
void softreset_receiver();

uint64_t get_sys_ts();
uint64_t get_rx_ts();
uint64_t get_tx_ts();

uint32 dwt_get_status();
void dwt_reset_status(uint32 status);

typedef void (*dwt_irq_func_t)(uint32);

extern dwt_irq_func_t _dwt_handler_cplock;
#define DWT_IRQ_CPLOCK SYS_STATUS_CPLOCK

extern dwt_irq_func_t _dwt_handler_esyncr;
#define DWT_IRQ_ESYNCR SYS_STATUS_ESYNCR

extern dwt_irq_func_t _dwt_handler_aat;
#define DWT_IRQ_AAT SYS_STATUS_AAT

extern dwt_irq_func_t _dwt_handler_send;
#define DWT_IRQ_SEND SYS_STATUS_TXFRS

extern dwt_irq_func_t _dwt_handler_rxfailed;
#define DWT_IRQ_RXFAILED (SYS_STATUS_RXPHE | SYS_STATUS_RXFCE | SYS_STATUS_RXPTO | SYS_STATUS_RXSFDTO | SYS_STATUS_AFFREJ | SYS_STATUS_LDEERR)

extern dwt_irq_func_t _dwt_handler_rxok;
#define DWT_IRQ_RXOK SYS_STATUS_RXFCG

extern dwt_irq_func_t _dwt_handler_rxtimeout;
#define DWT_IRQ_RXTIMEOUT SYS_STATUS_RXRFTO

extern dwt_irq_func_t _dwt_handler_rxoverrun;
#define DWT_IRQ_RXOVERRRUN SYS_STATUS_RXOVRR

extern dwt_irq_func_t _dwt_handler_sleep2init;
#define DWT_IRQ_SLEEP2INIT SYS_STATUS_SLP2INIT

extern dwt_irq_func_t _dwt_handler_pll_error;
#define DWT_IRQ_PLL_ERROR (SYS_STATUS_RFPLL_LL | SYS_STATUS_CLKPLL_LL)

extern dwt_irq_func_t _dwt_handler_hpdwarn;
#define DWT_IRQ_HPDWARN SYS_STATUS_HPDWARN


#define __DWT_IRQ_CALL_HANDLER(handler, status) if(handler) (*handler)(status);
void dwt_irq();

// -------------------- DEPRECATED --------------------
/* Length of the common part of the message (up to and including the function code, see NOTE 2 below). */
#define ALL_MSG_COMMON_LEN 10
/* Index to access some of the fields in the frames involved in the process. */
#define ALL_MSG_SEQ_NUM 2
#define ALL_MSG_TARGET 3
#define ALL_MSG_TYPE    9
#define FINAL_MSG_POLL_TX_TS_IDX 10
#define FINAL_MSG_RESP_RX_TS_IDX 14
#define FINAL_MSG_FINAL_TX_TS_IDX 18
#define FINAL_MSG_TS_LEN 4
#define ANGLE_MSG_IDX 10
#define LOCATION_FLAG_IDX 11
#define LOCATION_INFO_LEN_IDX 12
#define LOCATION_INFO_START_IDX 13
#define ANGLE_MSG_MAX_LEN 30

/* Timestamps of frames transmission/reception.
 * As they are 40-bit wide, we need to define a 64-bit int type to handle them. */

typedef signed long long int64;
typedef unsigned long long uint64;

#define TX_PGDELAY_CH5 0xC5
void configureTXPower(dwt_txconfig_t *config);
uint64 get_systime_u64(void);
uint64 get_tx_timestamp_u64(void);
uint64 get_rx_timestamp_u64(void);
void final_msg_get_ts(const uint8 *ts_field, uint32 *ts);
void final_msg_set_ts(uint8 *ts_field, uint64 ts);
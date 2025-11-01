#pragma once

#include "stdint.h"

#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_types.h"

#include "messages.h"


void dwt_custom_softReset();
void softreset_receiver();

int64_t get_sys_ts();
int64_t get_rx_ts();
int64_t get_tx_ts();

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
#define DWT_IRQ_RXFAILED (SYS_STATUS_AFFREJ | SYS_STATUS_RXPHE | SYS_STATUS_RXFCE | SYS_STATUS_RXRFSL | SYS_STATUS_RXPTO | SYS_STATUS_RXSFDTO | SYS_STATUS_LDEERR)

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


#define __DWT_IRQ_CALL_HANDLER(handler, status) if(handler) (*handler)(status)
void dwt_irq();

void dwt_showDiag(char* buf);

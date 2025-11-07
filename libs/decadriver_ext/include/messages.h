#pragma once

#include "stdint.h"
#include "stdio.h"
#include "string.h" /* memcpy */

#include "deca_regs.h"

typedef enum {
    MSG_PULL       = 0x11, ///< протокол 2-смс
    MSG_RESP       = 0x12, ///< протокол 2-смс
    MSG_PULL_3     = 0x21, ///< протокол 3-смс
    MSG_RESP_3     = 0x22, ///< протокол 3-смс
    MSG_FINAL      = 0x13, ///< протокол 3-смс
    MSG_DISTANCE   = 0x44
} MSG_Types;

#define MSG_PLACEHOLDER_8  0
#define MSG_PLACEHOLDER_16 0,0
#define MSG_PLACEHOLDER_32 0,0,0,0
#define MSG_PLACEHOLDER_TS MSG_PLACEHOLDER_32,MSG_PLACEHOLDER_8 ///< 40 bit
#define MSG_PLACEHOLDER_64 0,0,0,0,0,0,0,0

#define MSG_PLACEHOLDER_CTRL_normal 0x41,0x88
#define MSG_PLACEHOLDER_SEQNUM 0

/// ctrl, senum, pan_id, dest_id, src_id
#define MSG_HEADER_normal \
    MSG_PLACEHOLDER_CTRL_normal, MSG_PLACEHOLDER_SEQNUM, \
    MSG_PLACEHOLDER_16, MSG_PLACEHOLDER_16, MSG_PLACEHOLDER_16

#define MSG_HEADER_normal_len 9

#define MSG_TYPE_len 1

#define MSG_PLACEHOLDER_CRC MSG_PLACEHOLDER_16
#define MSG_CRC_len 2

#define MSG_PULL_len (MSG_HEADER_normal_len + MSG_TYPE_len + MSG_CRC_len)
extern uint8_t msg_pull[];
#define MSG_RESP_len (MSG_HEADER_normal_len + MSG_TYPE_len + 2*5 + MSG_CRC_len)
extern uint8_t msg_resp[];
#define MSG_FINAL_len (MSG_HEADER_normal_len + MSG_TYPE_len + 2*5 + MSG_CRC_len)
extern uint8_t msg_final[];

#define MSG_MAX_LEN MSG_RESP_len


#define MSG_SEQNUM(msg)  (msg[2])
#define MSG_PAN_ID(msg)  (*((uint16_t*) &msg[3])) //! тут вообще-то должен получаться невыровненый доступ
#define MSG_DEST_ID(msg) (*((uint16_t*) &msg[5]))
#define MSG_SRC_ID(msg)  (*((uint16_t*) &msg[7])) //! тут вообще-то должен получаться невыровненый доступ
#define MSG_TYPE(msg)    (msg[9])

#define MSG_RESP_ONE_pull_rx_ts_get(msg, dest_p) memcpy(dest_p, &msg[10], 5)
#define MSG_RESP_ONE_pull_rx_ts_set(msg, src_p) memcpy(&msg[10], src_p, 5)
#define MSG_RESP_ONE_resp_tx_ts_get(msg, dest_p) memcpy(dest_p, &msg[15], 5)
#define MSG_RESP_ONE_resp_tx_ts_set(msg, src_p) memcpy(&msg[15], src_p, 5)

/*
!           Про невыровненый доступ
! STM32F10xxx/20xxx/21xxx/L1xxxx Cortex®-M3 programming manual 3.3.5 [Address alignment]
! инструкция LDRH которая используется для доступа в uin16_t (PAN/DEST/SRC ID)
! поддерживает невыровненый доступ.
! Однако, LDRD требуется выравнивание (исп для 64 битных), поэтому возникала ошибка.
! Сейчас решена, т.к. исп memcpy.
!
! TODO: т.к. невыровненый доступ это медленнее и то что он вообще допустим не гарантировано
! стоит исправить.
*/


#define EVENTs_msg    0x1000
#define EVENTs_custom 0x2000
#define EVENTs_host   0x4000
#define EVENTs_dwt    0x5000

#define EVENT_is(event, event_class) ((event & 0xf000) == event_class)

#define MSG_TYPE_2_EVENT(type) (EVENTs_msg | type)

typedef enum{
    EVENT_none = 0,

    // ------------------------- MSG -------------------------

    EVENT_msg_PULL     = EVENTs_msg | MSG_PULL,
    EVENT_msg_RESP     = EVENTs_msg | MSG_RESP,
    EVENT_msg_PULL_3   = EVENTs_msg | MSG_PULL_3,
    EVENT_msg_RESP_3   = EVENTs_msg | MSG_RESP_3,
    EVENT_msg_FINAL    = EVENTs_msg | MSG_FINAL,
    EVENT_msg_DISTANCE = EVENTs_msg | MSG_DISTANCE,

    // ------------------------- CUSTOM -------------------------

    EVENT_initiate_ss_twr = EVENTs_custom | 0x1,
    EVENT_initiate_ds_twr = EVENTs_custom | 0x2,
    EVENT_initiate_sniffer  = EVENTs_custom | 0xf0,
    
    // ------------------------- HOST -------------------------
    
    // EVENT_timer

    // ------------------------- DWT -------------------------
    
    EVENT_rxtimeout = EVENTs_dwt | 0x0,
    EVENT_rxfail    = EVENTs_dwt | 0x10,
    EVENT_pll_error = EVENTs_dwt | 0xAA,
} MyEvents;

typedef uint16_t MsgEvent; // MyEvents | MSG_TYPE

uint8_t msgGetLen(MSG_Types msg_type);

MsgEvent toMsgEvent(uint32_t status, uint8_t msg_type, MyEvents ext);

char* showEvent(MyEvents event);

char* showMsgType(MSG_Types type);

/* 
MSG <TYPE>
    seq = <seq_num>
    dest_pan
    dest_addr
    src
    DATA

@param str_size используется в snprintf
*/
void showMsg(char* str, size_t str_size, uint8_t msg[]);

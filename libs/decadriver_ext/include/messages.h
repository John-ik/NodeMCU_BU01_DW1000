#pragma once

#include "stdint.h"
#include "stdio.h"
#include "string.h" /* memcpy */

#include "deca_types.h"
#include "deca_regs.h"

typedef enum {
    MSG_PULL_ONE = 0x01,
    MSG_RESP_ONE = 0x02,
    MSG_PULL     = 0x11,
    MSG_RESPONSE = 0x22,
    MSG_FINAL    = 0x33,
    MSG_DISTANCE = 0x44,
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

#define MSG_PULL_ONE_len (MSG_HEADER_normal_len + MSG_TYPE_len + MSG_CRC_len)
extern uint8 msg_pull_one[];
#define MSG_RESP_ONE_len (MSG_HEADER_normal_len + MSG_TYPE_len + 2*5 + MSG_CRC_len)
extern uint8 msg_resp_one[];

#define MSG_MAX_LEN MSG_RESP_ONE_len


#define MSG_SEQNUM(msg)  (msg[2])
#define MSG_PAN_ID(msg)  (*((uint16*) &msg[3]))
#define MSG_DEST_ID(msg) (*((uint16*) &msg[5]))
#define MSG_SRC_ID(msg)  (*((uint16*) &msg[7]))
#define MSG_TYPE(msg)    (msg[9])

#define MSG_RESP_ONE_pull_rx_ts_get(msg, dest_p) memcpy(dest_p, &msg[10], 5)
#define MSG_RESP_ONE_pull_rx_ts_set(msg, src_p) memcpy(&msg[10], src_p, 5)
#define MSG_RESP_ONE_resp_tx_ts_get(msg, dest_p) memcpy(dest_p, &msg[15], 5)
#define MSG_RESP_ONE_resp_tx_ts_set(msg, src_p) memcpy(&msg[15], src_p, 5)




#define EVENTs_msg    0x1000
#define EVENTs_custom 0x2000
#define EVENTs_host   0x4000
#define EVENTs_dwt    0x5000

#define EVENT_is(event, event_class) ((event & 0xf000) == event_class)

#define MSG_TYPE_2_EVENT(type) (EVENTs_msg | type)

typedef enum{
    EVENT_none = 0,

    // ------------------------- MSG -------------------------

    EVENT_msg_PULL_ONE = EVENTs_msg | MSG_PULL_ONE,
    EVENT_msg_RESP_ONE = EVENTs_msg | MSG_RESP_ONE,
    EVENT_msg_PULL     = EVENTs_msg | MSG_PULL,
    EVENT_msg_RESPONSE = EVENTs_msg | MSG_RESPONSE,
    EVENT_msg_FINAL    = EVENTs_msg | MSG_FINAL,
    EVENT_msg_DISTANCE = EVENTs_msg | MSG_DISTANCE,

    // ------------------------- CUSTOM -------------------------

    EVENT_initiate_pull_one = EVENTs_custom | 0x0,
    
    // ------------------------- HOST -------------------------
    
    // EVENT_timer

    // ------------------------- DWT -------------------------
    
    EVENT_rxtimeout = EVENTs_dwt | 0x0,
    EVENT_rxfail    = EVENTs_dwt | 0x10,
    EVENT_pll_error = EVENTs_dwt | 0xAA,
} MyEvents;

typedef uint16 MsgEvent; // MyEvents | MSG_TYPE

MsgEvent toMsgEvent(uint32 status, uint8 msg_type, MyEvents ext);

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
void showMsg(char* str, size_t str_size, uint8 msg[]);

#pragma once

#include "stdint.h"
#include "stdio.h"

#include "deca_types.h"
#include "deca_regs.h"

#define MSG_PULL_ONE 0x01
#define MSG_RESP_ONE 0x02
#define MSG_PULL     0x11
#define MSG_RESPONSE 0x22
#define MSG_FINAL    0x33
#define MSG_DISTANCE 0x44
#define MSG_ERROR_TX 0xF1
#define MSG_ERROR_RX 0xF2

typedef enum{
    EVENT_none = 0,

    EVENT_msg_PULL_ONE = MSG_PULL_ONE,
    EVENT_msg_RESP_ONE = MSG_RESP_ONE,
    EVENT_msg_PULL = MSG_PULL,
    EVENT_msg_RESPONSE = MSG_RESPONSE,
    EVENT_msg_FINAL = MSG_FINAL,
    EVENT_msg_DISTANCE = MSG_DISTANCE,
    EVENT_msg_ERROR_TX = MSG_ERROR_TX,
    EVENT_msg_ERROR_RX = MSG_ERROR_RX,

    EVENT_rxtimeout = 0x200,

    EVENT_initiate_pull_one = 0x300,
    
    EVENT_pll_error = 0x500,
    // EVENT_timer
} MyEvents;

typedef uint16 MsgEvent; // MyEvents | MSG_TYPE

MsgEvent toMsgEvent(uint32 status, uint8 msg_type, MyEvents ext);


#define MSG_DATA_MAX_LEN  24
#define MSG_MAX_LEN (MSG_DATA_MAX_LEN + 12)


typedef union {
    uint8 _empty;
    struct {
        uint64_t pull_rx_ts;
        uint64_t resp_tx_ts;
    } resp_one;

    /**
     * rx_code: check MSG_ERROR_RX_CODE_...
     * tx_code: check MSG_ERROR_TX_CODE_...
     * 
     * field:
     *      RX -> UNDEFINED_TYPE -> type received byte
     *      RX -> NOT_EQUAL_LEN  -> receive len into 16:31 bits; i into 0:15
     */
    struct {
        uint8 rx_code;
        uint8 tx_code;
        uint32 field; 
    } error;
    
} Msg_Data;

#define MSG_DATA_EMPTY {._empty=0}

typedef struct {
    uint16 frame_control;
    uint8  seq_num;
    uint16 dest_pan;
    uint16 dest_addr;
    uint16 src_addr;
    uint8  type;
    Msg_Data data;
    uint16 _crc;
} MacMessage;

#define MAC_MESSAGE_create(ctrl, msg_type) {   \
        .frame_control=ctrl,                   \
        .seq_num=0,                            \
        .dest_pan=0,                           \
        .dest_addr=0,                          \
        .src_addr=0,                           \
        .type=msg_type,                        \
        .data=MSG_DATA_EMPTY,                  \
        ._crc=0                                \
    }

#define MSG_ERROR_RX_CODE_UNDEFINED_TYPE 0x01
#define MSG_ERROR_RX_CODE_NOT_EQUAL_LEN  0x02

extern MacMessage error_rx_msg;
extern MacMessage error_tx_msg;


#define WRITEMSG2BYTES_1(out, what, i) \
    *(out + i++) = what & 0xFF

#define WRITEMSG2BYTES_2(out, what, i) \
    *(out + i++) = (uint8)((what & 0xFF00) >> 8); \
    *(out + i++) = what & 0xFF

#define WRITEMSG2BYTES_4(out, what, i) \
    *(out + i++) = (uint8)((what & 0xFF000000) >> 24); \
    *(out + i++) = (uint8)((what & 0xFF0000) >> 16); \
    *(out + i++) = (uint8)((what & 0xFF00) >> 8); \
    *(out + i++) = what & 0xFF

#define WRITEMSG2BYTES_5(out, what, i) \
    *(out + i++) = (uint8)((what & 0xFF00000000) >> 32); \
    *(out + i++) = (uint8)((what & 0xFF000000) >> 24); \
    *(out + i++) = (uint8)((what & 0xFF0000)>> 16); \
    *(out + i++) = (uint8)((what & 0xFF00) >> 8); \
    *(out + i++) = what & 0xFF

uint16 msg2bytes(MacMessage msg, uint8 out[MSG_MAX_LEN]);

#define READBYTES2MSG_1(input, i) \
    (input[i++])

#define READBYTES2MSG_2(input, i) \
    ((input[i] << 8) | (input[i+1])); \
    i += 2

#define READBYTES2MSG_4(input, i) \
    ((input[i] << 24) | (input[i+1] << 16) | (input[i+2] << 8) | (input[i+3])); \
    i += 4

#define READBYTES2MSG_5(input, i) \
    (((uint64_t) input[i] << 32) |(input[i+1] << 24) | (input[i+2] << 16) | (input[i+3] << 8) | (input[i+4])); \
    i += 5

MacMessage bytes2msg(uint8 input[MSG_MAX_LEN], uint16 msg_len);


char* showEvent(MyEvents event);

/* 
str.length > 110

MSG <TYPE>
    ctrl = <frame_control>
    seq = <seq_num>
    dest_pan
    dest_addr
    src
    DATA

*/
void showMsg(char* str, MacMessage msg);
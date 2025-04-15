#pragma once

#include "stdio.h"

#include "deca_types.h"
#include "deca_regs.h"

typedef enum{
    EVENT_none = 0,
    // EVENT_rxgood = 0x100, // If MSG_TYPE in event then its rxgood
    EVENT_rxtimeout = 0x200,
    EVENT_initiate_pull_one = 0x300,
    // EVENT_timer
} MyEvents;

typedef uint16 MsgEvent; // MyEvents | MSG_TYPE

MsgEvent toMsgEvent(uint32 status, uint8 msg_type, MyEvents ext){
    if (status & SYS_STATUS_RXFCG){ // RX GOOD
        return msg_type;
    }
    if (status & SYS_STATUS_RXRFTO){ // RX TIMEOUT
        return EVENT_rxtimeout;
    }
    return ext;
}


#define MSG_DATA_MAX_LEN  24
#define MSG_MAX_LEN (MSG_DATA_MAX_LEN + 12)

#define MSG_PULL_ONE 0x01
#define MSG_RESP_ONE 0x02
#define MSG_PULL     0x11
#define MSG_RESPONSE 0x22
#define MSG_FINAL    0x33
#define MSG_DISTANCE 0x44

typedef union {
    uint8 _empty;
    struct {
        uint32 pull_rx_ts;
        uint32 resp_tx_ts;
    } resp_one;
} Msg_Data;

#define MSG_DATA_EMPTY (const Msg_Data){._empty=0}

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

#define WRITEMSG2BYTES_1(out, what, i) \
    *(out + i++) = what & 0xFF

#define WRITEMSG2BYTES_2(out, what, i) \
    *(out + i++) = (uint8)((what & 0xFF00) >> 8); *(out + i++) = what & 0xFF

#define WRITEMSG2BYTES_4(out, what, i) \
    *(out + i++) = (uint8)((what & 0xFF000000) >> 24); *(out + i++) = (uint8)((what & 0xFF0000)>>16); \
    *(out + i++) = (uint8)((what & 0xFF00) >> 8); *(out + i++) = what & 0xFF


uint16 msg2bytes(MacMessage msg, uint8 out[MSG_MAX_LEN]){
    uint16 i = 0;
    WRITEMSG2BYTES_2(out, msg.frame_control, i);
    WRITEMSG2BYTES_1(out, msg.seq_num, i);
    WRITEMSG2BYTES_2(out, msg.dest_pan, i);
    WRITEMSG2BYTES_2(out, msg.dest_addr, i);
    WRITEMSG2BYTES_2(out, msg.src_addr, i);
    WRITEMSG2BYTES_1(out, msg.type, i);
    switch (msg.type){
        case MSG_RESP_ONE:
            WRITEMSG2BYTES_4(out, msg.data.resp_one.pull_rx_ts, i);
            WRITEMSG2BYTES_4(out, msg.data.resp_one.resp_tx_ts, i);
            break;

        case MSG_PULL_ONE:
        case MSG_PULL:
            break;
        default:
            return 0;
    }
    WRITEMSG2BYTES_2(out, 0, i); // crc zeros
    return i;
}

MacMessage bytes2msg(uint8 input[MSG_MAX_LEN], uint16 msg_len){
    MacMessage msg; uint16 i = 0;
    msg.frame_control = ((uint16)input[0] << 8) | input[1]; i += 2;
    msg.seq_num = input[i++];
    msg.dest_pan = ((uint16)input[i] << 8) | input[i+1]; i += 2;
    msg.dest_addr= ((uint16)input[i] << 8) | input[i+1]; i += 2;
    msg.src_addr = ((uint16)input[i] << 8) | input[i+1]; i += 2;
    msg.type = input[i++];
    switch(msg.type){
        case MSG_RESP_ONE:
            msg.data.resp_one.pull_rx_ts = ((uint32)input[i] << 24)|((uint32)input[i+1] << 16)|((uint32)input[i+2] << 8)|(input[i+3]); i+= 3;
            msg.data.resp_one.resp_tx_ts = ((uint32)input[i] << 24)|((uint32)input[i+1] << 16)|((uint32)input[i+2] << 8)|(input[i+3]); i+= 3;
            break;
        default:
            break;
    }
    msg._crc = (input[msg_len-1] << 8) | (input[msg_len]);
    return msg;
}


char* showEvent(MyEvents event){
    char* default_str = "! UNDEFINED EVENT 0x%02X !";

    switch(event){
        case EVENT_none:              return "EVENT_none";
        case EVENT_rxtimeout:         return "EVENT_rxtimeout";
        case EVENT_initiate_pull_one: return "EVENT_initiate_pull_one";

        case MSG_PULL_ONE: return "EVENT_msg_pull_one";
        case MSG_RESP_ONE: return "EVENT_msg_resp_one";
        case MSG_PULL:     return "EVENT_msg_pull";
        case MSG_RESPONSE: return "EVENT_msg_response";
        case MSG_FINAL:    return "EVENT_msg_final";
        case MSG_DISTANCE: return "EVENT_msg_distance";

        default: 
            sprintf(default_str, default_str, event);
            return default_str;
    }
}

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
void showMsg(char* str, MacMessage msg){
    char data_buf[256] = "";

    switch(msg.type){
        case MSG_PULL_ONE:
            break;
        case MSG_RESP_ONE:
            sprintf(data_buf,   "\tpull_rx_ts = %u\n"
                                "\tresp_tx_ts = %u",
                msg.data.resp_one.pull_rx_ts, msg.data.resp_one.resp_tx_ts);
            break;
        default:
            return;
    }

    char bytes_s[2*MSG_MAX_LEN];
    uint8 bytes[MSG_MAX_LEN];
    uint16 frame_len = msg2bytes(msg, bytes);

    for(size_t i = 0; i < frame_len; i++){
        sprintf(bytes_s + 2*i, "%02X", bytes[i]);
    }

    sprintf(str, "MSG %s\n"
                    "\tctrl = 0x%X\n"
                    "\tseq = %d\n"
                    "\tdest_pan = 0x%X\n"
                    "\tdest_addr = 0x%X\n"
                    "\tsrc = 0x%X\n"
                    "%s\n"
                    "RAW: 0x%s",

        showEvent(msg.type), msg.frame_control, msg.seq_num, msg.dest_pan, msg.dest_addr, msg.src_addr, data_buf, bytes_s
    );
}

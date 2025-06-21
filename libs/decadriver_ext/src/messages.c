#include "messages.h"

MacMessage error_rx_msg = MAC_MESSAGE_create(0x4188, MSG_ERROR_RX);
MacMessage error_tx_msg = MAC_MESSAGE_create(0x4188, MSG_ERROR_TX);


MsgEvent toMsgEvent(uint32 status, uint8 msg_type, MyEvents ext){
    if (status & SYS_STATUS_RXFCG){ // RX GOOD
        return msg_type;
    }
    if (status & SYS_STATUS_RXRFTO){ // RX TIMEOUT
        return EVENT_rxtimeout;
    }
    return ext;
}

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
            WRITEMSG2BYTES_5(out, msg.data.resp_one.pull_rx_ts, i);
            WRITEMSG2BYTES_5(out, msg.data.resp_one.resp_tx_ts, i);
            break;

        case MSG_PULL_ONE:
        case MSG_PULL:
            break;

        case MSG_ERROR_RX:
        case MSG_ERROR_TX:
            WRITEMSG2BYTES_4(out, 0xDEADBEEF, i);
            break;

        default:
            return 0;
    }
    WRITEMSG2BYTES_2(out, 0, i); // crc zeros
    return i;
}

MacMessage bytes2msg(uint8 input[MSG_MAX_LEN], uint16 msg_len){
    MacMessage msg; uint16 i = 0;
    msg.frame_control = READBYTES2MSG_2(input, i);
    msg.seq_num = READBYTES2MSG_1(input, i);
    msg.dest_pan = READBYTES2MSG_2(input, i);
    msg.dest_addr= READBYTES2MSG_2(input, i);
    msg.src_addr = READBYTES2MSG_2(input, i);
    msg.type = READBYTES2MSG_1(input, i);
    switch(msg.type){
        case MSG_RESP_ONE:
            msg.data.resp_one.pull_rx_ts = READBYTES2MSG_5(input, i);
            msg.data.resp_one.resp_tx_ts = READBYTES2MSG_5(input, i);
            break;
        case MSG_PULL_ONE:
            break;
        default:
            msg.data.error.rx_code = MSG_ERROR_RX_CODE_UNDEFINED_TYPE;
            msg.data.error.field   = msg.type;
            msg.type = MSG_ERROR_RX;
            break;
    }

    msg._crc = READBYTES2MSG_2(input, i);
    if (msg_len == i)
        return msg;
    else{
        msg.type = MSG_ERROR_RX;
        msg.data.error.rx_code = MSG_ERROR_RX_CODE_NOT_EQUAL_LEN;
        msg.data.error.field   = (msg_len << 16) | (i);
        return error_tx_msg;
    }
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
        case MSG_ERROR_RX: return "! EVENT_msg_ERROR_RX !";
        case MSG_ERROR_TX: return "! EVENT_msg_ERROR_TX !";

        default: 
            sprintf(default_str, default_str, event);
            return default_str;
    }
}

void showMsg(char* str, MacMessage msg){
    char data_buf[256] = "";

    switch(msg.type){
        case MSG_PULL_ONE:
            break;
        case MSG_RESP_ONE:
            sprintf(data_buf,   "\tpull_rx_ts = 0x%lX%02X\n"
                                "\tresp_tx_ts = 0x%lX%02X",
                (uint32_t) msg.data.resp_one.pull_rx_ts >> 8, (uint8_t) msg.data.resp_one.pull_rx_ts & 0xff,
                (uint32_t) msg.data.resp_one.resp_tx_ts >> 8, (uint8_t) msg.data.resp_one.resp_tx_ts & 0xff
            );
            break;
        case MSG_ERROR_RX:
            sprintf(data_buf,   "\trx_code = 0x%X"
                                "\tfield = 0x%X", 
                    msg.data.error.rx_code, msg.data.error.field);
            break;
        case MSG_ERROR_TX:
            sprintf(data_buf,   "\ttx_code = 0x%X"
                                "\tfield = 0x%X", 
                    msg.data.error.tx_code, msg.data.error.field);
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
                    "\tseq = %u\n"
                    "\tdest_pan = 0x%X\n"
                    "\tdest_addr = 0x%X\n"
                    "\tsrc = 0x%X\n"
                    "%s\n"
                    "RAW: 0x%s",

        showEvent(msg.type), msg.frame_control, msg.seq_num, msg.dest_pan, msg.dest_addr, msg.src_addr, data_buf, bytes_s
    );
}

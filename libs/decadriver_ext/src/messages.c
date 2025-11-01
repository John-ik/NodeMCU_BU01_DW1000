#include "messages.h"

#define MSG_BEGIN MSG_HEADER_normal
#define MSG_END   MSG_PLACEHOLDER_CRC

uint8_t msg_pull_one[MSG_PULL_ONE_len] = {MSG_BEGIN, MSG_PULL_ONE, MSG_END};
uint8_t msg_resp_one[MSG_RESP_ONE_len] = {MSG_BEGIN, MSG_RESP_ONE, MSG_PLACEHOLDER_TS, MSG_PLACEHOLDER_TS, MSG_END};

#undef MSG_BEGIN
#undef MSG_END

uint8_t msgGetLen(MSG_Types msg_type){
    switch(msg_type){
#define X(x) case x: return x##_len
        X(MSG_PULL_ONE);
        X(MSG_RESP_ONE);
#undef X
        default:
            return 0;
    }
}


MsgEvent toMsgEvent(uint32_t status, uint8_t msg_type, MyEvents ext){
    if (status & SYS_STATUS_RXFCG){ // RX GOOD
        return msg_type;
    }
    if (status & SYS_STATUS_RXRFTO){ // RX TIMEOUT
        return EVENT_rxtimeout;
    }
    return ext;
}

char* showEvent(MyEvents event){
    char* default_str = "! UNDEFINED EVENT 0x%04X !";

    switch(event){
        case EVENT_none:              return "EVENT_none";
        
        case EVENT_msg_PULL_ONE: return "EVENT_msg_pull_one";
        case EVENT_msg_RESP_ONE: return "EVENT_msg_resp_one";
        case EVENT_msg_PULL:     return "EVENT_msg_pull";
        case EVENT_msg_RESPONSE: return "EVENT_msg_response";
        case EVENT_msg_FINAL:    return "EVENT_msg_final";
        case EVENT_msg_DISTANCE: return "EVENT_msg_distance";
        
        case EVENT_initiate_pull_one: return "EVENT_initiate_pull_one";
        
        case EVENT_rxtimeout:         return "EVENT_rxtimeout";
        
        case EVENT_pll_error:         return "EVENT_pll_error";

        default: 
            sprintf(default_str, default_str, event);
            return default_str;
    }
}

char* showMsgType(MSG_Types type){
    switch(type){
        case MSG_PULL_ONE: return "MSG_pull_one";
        case MSG_RESP_ONE: return "MSG_resp_one";
        case MSG_PULL:     return "MSG_pull";
        case MSG_RESPONSE: return "MSG_response";
        case MSG_FINAL:    return "MSG_final";
        case MSG_DISTANCE: return "MSG_distance";
    }
    return "! undefined msg type !";
}

void showMsg(char* str, size_t str_size, uint8_t msg[]){
    uint8_t frame_len = 12;

    uint8_t msg_type = MSG_TYPE(msg);

    int printed = snprintf(str, str_size,
                    "MSG> %s\n"
                    "\tseq = %u\n"
                    "\tdest_pan = 0x%X\n"
                    "\tdest_addr = 0x%X\n"
                    "\tsrc = 0x%X\n",
        showMsgType(msg_type),
        MSG_SEQNUM(msg), MSG_PAN_ID(msg), MSG_DEST_ID(msg), MSG_SRC_ID(msg)
    );
    str += printed;
    str_size -= printed;

    static uint64_t pull_rx_ts, resp_tx_ts;

    switch(msg_type){
        case MSG_PULL_ONE:
            break;
        case MSG_RESP_ONE:
            MSG_RESP_ONE_pull_rx_ts_get(msg, &pull_rx_ts);
            MSG_RESP_ONE_resp_tx_ts_get(msg, &resp_tx_ts);
    #if __IMPORTC__
            printed = snprintf(str, str_size,
                                "\tpull_rx_ts = 0x%010llX\n"
                                "\tresp_tx_ts = 0x%010llX\n",
                                pull_rx_ts,
                                resp_tx_ts
            );
    #else
            printed = snprintf(str, str_size,
                                "\tpull_rx_ts = 0x%02X%08lX\n"
                                "\tresp_tx_ts = 0x%02X%08lX\n",
                                (uint8_t)(pull_rx_ts >> 32), (uint32_t) pull_rx_ts,
                                (uint8_t)(resp_tx_ts >> 32), (uint32_t) resp_tx_ts
            );
    #endif
            goto printed;
        default:
            return;

        printed:
            str += printed;
            str_size -= printed;
    }
    frame_len = msgGetLen(msg_type); // если тип невалиден, то будет return в свиче
    
    printed = snprintf(str, str_size, "RAW: ");
    str += printed;
    str_size -= printed;

    for(size_t i = 0; i < frame_len; i++){
        printed = snprintf(str, str_size, "%02X", msg[i]);
        str += printed;
        str_size -= printed;
    }
}

#include "messages.h"

#define MSG_BEGIN MSG_HEADER_normal
#define MSG_END   MSG_PLACEHOLDER_CRC

uint8_t msg_pull[MSG_PULL_len]   = {MSG_BEGIN, MSG_PULL, MSG_END};
uint8_t msg_resp[MSG_RESP_len]   = {MSG_BEGIN, MSG_RESP, MSG_PLACEHOLDER_TS, MSG_PLACEHOLDER_TS, MSG_END};
uint8_t msg_final[MSG_FINAL_len] = {MSG_BEGIN, MSG_FINAL, MSG_PLACEHOLDER_TS, MSG_PLACEHOLDER_TS, MSG_PLACEHOLDER_TS, MSG_END};
uint8_t msg_dist[MSG_DIST_len]   = {MSG_BEGIN, MSG_DIST, MSG_PLACEHOLDER_DIST, MSG_END};
//TODO: вместо tof, сразу передавать fixed-point с коэф 2^5

#undef MSG_BEGIN
#undef MSG_END

uint8_t msgGetLen(MSG_Types msg_type){
    switch(msg_type){
#define X(x) case x: return x##_len
        case MSG_PULL_3:
        X(MSG_PULL);

        case MSG_RESP_3:
        X(MSG_RESP);

        X(MSG_FINAL);

        X(MSG_DIST);
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
        
        case EVENT_msg_PULL: return "EVENT_msg_pull";
        case EVENT_msg_RESP: return "EVENT_msg_resp";
        case EVENT_msg_PULL_3:     return "EVENT_msg_pull_3";
        case EVENT_msg_RESP_3: return "EVENT_msg_resp_3";
        case EVENT_msg_FINAL:    return "EVENT_msg_final";
        case EVENT_msg_DIST: return "EVENT_msg_distance";
        
        case EVENT_initiate_pull: return "EVENT_initiate_pull";
        
        case EVENT_rxtimeout:         return "EVENT_rxtimeout";
        
        case EVENT_pll_error:         return "EVENT_pll_error";

        default: 
            sprintf(default_str, default_str, event);
            return default_str;
    }
}

char* showMsgType(MSG_Types type){
    switch(type){
        case MSG_PULL: return "MSG_pull";
        case MSG_RESP: return "MSG_resp";
        case MSG_PULL_3:     return "MSG_pull_3";
        case MSG_RESP_3: return "MSG_resp_3";
        case MSG_FINAL:    return "MSG_final";
        case MSG_DIST: return "MSG_distance";
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

    static uint64_t rx_ts, tx_ts;
    static float dist;

    switch(msg_type){
        case MSG_PULL:
        case MSG_PULL_3:
            break;
        case MSG_RESP:
        case MSG_RESP_3:
            MSG_RESP_pull_rx_ts_get(msg, &rx_ts);
            MSG_RESP_resp_tx_ts_get(msg, &tx_ts);
    #if __IMPORTC__
            printed = snprintf(str, str_size,
                                "\tpull_rx_ts = 0x%010llX\n"
                                "\tresp_tx_ts = 0x%010llX\n",
                                rx_ts,
                                tx_ts
            );
    #else
            printed = snprintf(str, str_size,
                                "\tpull_rx_ts = 0x%02X%08lX\n"
                                "\tresp_tx_ts = 0x%02X%08lX\n",
                                (uint8_t)(rx_ts >> 32), (uint32_t) rx_ts,
                                (uint8_t)(tx_ts >> 32), (uint32_t) tx_ts
            );
    #endif
            goto printed;
        
        case MSG_FINAL:
            MSG_RESP_pull_rx_ts_get(msg, &rx_ts);
            MSG_RESP_resp_tx_ts_get(msg, &tx_ts);
    #if __IMPORTC__
            printed = snprintf(str, str_size,
                                "\tresp_rx_ts = 0x%010llX\n"
                                "\tfinal_tx_ts = 0x%010llX\n",
                                rx_ts,
                                tx_ts
            );
    #else
            printed = snprintf(str, str_size,
                                "\tresp_rx_ts = 0x%02X%08lX\n"
                                "\tfinal_tx_ts = 0x%02X%08lX\n",
                                (uint8_t)(rx_ts >> 32), (uint32_t) rx_ts,
                                (uint8_t)(tx_ts >> 32), (uint32_t) tx_ts
            );
    #endif
            goto printed;
        
        case MSG_DIST:
            MSG_DIST_dist_get(msg, &dist);
            printed = snprintf(str, str_size,
                                "\tdist = %.2f\n",
                                dist
            );
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

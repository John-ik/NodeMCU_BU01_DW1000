#include "mac.h"

void set_uint16 (uint8* bytes, uint16 two){
    bytes[0] = (two & 0xFF00) >> 8;
    bytes[1] = two & 0x00FF;
}

uint16 get_uint16 (uint8* bytes){
    return (bytes[1] << 8) | bytes[0];
}

int MAC_header_to_bytes (MAC_Header header, uint8* bytes){
    int i = 0;
    set_uint16(bytes, header.frame_control);
    i = 2;
    bytes[i++] = header.seq_number;
    
    if (header.frame_control & MAC_FRAME_mask_PAN_ID){
        if (header.dest_addr && header.source_addr){
            if (header.dest_pan_id){
                set_uint16(bytes+i, header.dest_pan_id);
                i += 2;
            }
        }else
            return -1;
    }else{
        if (header.dest_addr && header.dest_pan_id){
            set_uint16(bytes+i, header.dest_pan_id); i += 2;
            set_uint16(bytes+i, header.dest_addr);   i += 2;
        }else
            return -1;

        if (header.source_addr && header.source_pan_id){
            set_uint16(bytes+i, header.source_pan_id); i += 2;
            set_uint16(bytes+i, header.source_addr);   i += 2;
        }else
            return -1;
    }

    for (int ii = 0; ii < 14; ii++){
        bytes[(i++)+ii] = header.aux_secure_header[ii];
    }


    return i;
}


MAC_Header MAC_bytes_2_header (uint8* bytes){
    MAC_Header head; int i = 0;

    head.frame_control = get_uint16(bytes); i += 2;
    head.seq_number = bytes[i++];

    if (head.frame_control & MAC_FRAME_mask_DEST_ADDR){

    }

    return head;
}

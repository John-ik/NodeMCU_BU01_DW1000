#pragma once

#include "deca_types.h"

#define MAC_FRAME_mask_TYPE   0x0007
#define MAC_FRAME_mask_SECURE 0x0008
#define MAC_FRAME_mask_FRAME_PENDING 0x0010
#define MAC_FRAME_mask_ACK_REQ       0x0020
#define MAC_FRAME_mask_PAN_ID        0x0040
#define MAC_FRAME_mask_DEST_ADDR     0x0C00
#define MAC_FRAME_mask_VERSION       0x3000
#define MAC_FRAME_mask_SOURCE_ADDR   0xC000

typedef struct {
    uint16 frame_control;
    uint8  seq_number;
    uint16 dest_pan_id;
    uint16 dest_addr;
    uint16 source_pan_id;
    uint16 source_addr;
    uint8  aux_secure_header[14];
} MAC_Header;


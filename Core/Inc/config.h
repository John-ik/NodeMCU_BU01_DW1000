#pragma once

// Indexing TAGs and Anchor from 1. 0 is special
#define TAG
// #define ANCHOR

#define MY_PAN_ID 0x0010 // ID of network

/*
 new ID system TODO:
 addr 0xA*** - Anchor
 addr 0xB*** - Tag
*/
#define ANCHOR_ID_marker 0xA000
#define TAG_ID_marker    0xB000

#ifdef TAG
  char whoami[] = "TAG";
  uint16 my_addr = TAG_ID_marker | COMPILE_ID;
#endif
#ifdef ANCHOR
  char whoami[] = "ANCHOR";
  uint16 my_addr = ANCHOR_ID_marker | COMPILE_ID;
#endif
#ifdef TAG
  #ifdef ANCHOR
    #error "WHO AM I ???"
  #endif
#endif
#ifndef TAG
  #ifndef ANCHOR
    #error "WHO AM I ???"
  #endif
#endif

#define DEBUG_DWT_DIAG
#define DEBUG_DWT_DIAG_TIMEOUT 10000

/* Default antenna delay values for 64 MHz PRF. See NOTE 1 below. */
#define TX_ANT_DLY 16436
#define RX_ANT_DLY 16436
// #define TX_ANT_DLY 0
// #define RX_ANT_DLY 32950

dwt_config_t config =
{
    5,               /* Channel number. */
    DWT_PRF_16M,     /* Pulse repetition frequency. */
    DWT_PLEN_2048,   /* Preamble length. */
    DWT_PAC8,       /* Preamble acquisition chunk size. Used in RX only. */
    4,               /* TX preamble code. Used in TX only. */
    4,               /* RX preamble code. Used in RX only. */
    0,               /* Use non-standard SFD (Boolean) */
    DWT_BR_110K,     /* Data rate. */
    DWT_PHRMODE_STD, /* PHY header mode. */
    0 // (2048 + 1 + 64 - 8) // (2048 + 1 + 64 - 8) /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
};


/** UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) conversion factor.
 * 1 uus = 512 / 499.2 ? and 1 ? = 499.2 * 128 dtu.
 * ! number is arounded to easy optimize multiple 
 */
#define UUS_TO_DWT_TIME 65536

/* Delay between frames, in UWB microseconds. See NOTE 4 below. */
/* This is the delay from Frame RX timestamp to TX reply timestamp used for calculating/setting the DW1000's delayed TX function. This includes the
 * frame length of approximately 2.46 ms with above configuration. */
#define POLL_RX_TO_RESP_TX_DLY_UUS 2600
/* This is the delay from the end of the frame transmission to the enable of the receiver, as programmed for the DW1000's wait for response feature. */
#define RESP_TX_TO_FINAL_RX_DLY_UUS 500
/* Receive final timeout. See NOTE 5 below. */
#define FINAL_RX_TIMEOUT_UUS 3300


/* Delay between frames, in UWB microseconds. See NOTE 4 below. */
/* This is the delay from the end of the frame transmission to the enable of the receiver, as programmed for the DW1000's wait for response feature. */
#define POLL_TX_TO_RESP_RX_DLY_UUS 50
/* This is the delay from Frame RX timestamp to TX reply timestamp used for calculating/setting the DW1000's delayed TX function. This includes the
 * frame length of approximately 2.66 ms with above configuration. */
#define RESP_RX_TO_FINAL_TX_DLY_UUS 2800 //2700 will fail
/* Receive response timeout. See NOTE 5 below. */
#define RESP_RX_TIMEOUT_UUS 2700


#define REQUEST_TO_RESPONSE_DELAY 0


/* Speed of light in air, in metres per second. */
#ifndef SPEED_OF_LIGHT
#define SPEED_OF_LIGHT 299702547
#endif




// ==================== PROTOLOCS CONFIG ====================

#define INITIATE_PULL_ONE_TIMEOUT_MS 5000

#define PULL_ONE_TIMEOUT_US 10000
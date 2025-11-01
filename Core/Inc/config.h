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

#define TRACE_MSG_ON

/* Default antenna delay values for 64 MHz PRF. See NOTE 1 below. */
#define TX_ANT_DLY 16436
#define RX_ANT_DLY 16436

dwt_config_t config =
{
    5,               /* Channel number. */
    DWT_PRF_16M,     /* Pulse repetition frequency. */
    DWT_PLEN_128,   /* Preamble length. */
    DWT_PAC8,       /* Preamble acquisition chunk size. Used in RX only. */
    4,               /* TX preamble code. Used in TX only. */
    4,               /* RX preamble code. Used in RX only. */
    0,               /* Use non-standard SFD (Boolean) */
    DWT_BR_6M8,     /* Data rate. */
    DWT_PHRMODE_STD, /* PHY header mode. */
    0 // (2048 + 1 + 64 - 8) // (2048 + 1 + 64 - 8) /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
};


/** UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) conversion factor.
 * 1 uus = 512 / 499.2 ? and 1 ? = 499.2 * 128 dtu.
 * ! number is arounded to easy optimize multiple 
 */
#define UUS_TO_DWT_TIME 65536uLL


/* Speed of light in air, in metres per second. */
#ifndef SPEED_OF_LIGHT
#define SPEED_OF_LIGHT 299702547
#endif




// ==================== PROTOLOCS CONFIG ====================

#define DEFAULT_RX_TIMEOUT_UUS      500

#define INITIATE_PULL_ONE_TIMEOUT_MS 100


/// инициация со стороны тэга и теперь тэг ждет столько до начала RX чтобы поймать RESP
#define POLL_TX_TO_RESP_RX_DLY_UUS  2000uLL
/// якорь поймал POLL и отправит RESP через
#define POLL_RX_TO_RESP_TX_DLY_UUS  2100uLL
/// тэг ждёт в течении
#define RESP_RX_TIMEOUT_UUS         DEFAULT_RX_TIMEOUT_UUS

/// якорь ждет после RESP чтобы начать RX для FINAL
#define RESP_TX_TO_FINAL_RX_DLY_UUS 2000uLL
/// тэг поймал RESP и отправит FINAL через
#define RESP_RX_TO_FINAL_TX_DLY_UUS 21000uLL
/// якорь ждёт
#define FINAL_RX_TIMEOUT_UUS        DEFAULT_RX_TIMEOUT_UUS


// #define PRE_TIMEOUT 8

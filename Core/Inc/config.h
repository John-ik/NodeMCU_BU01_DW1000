#pragma once

// Indexing TAGs and Anchor from 1. 0 is special
// #define TAG
#define ANCHOR

// #define SNIFFER_FOR_DEBUG

#define MY_PAN_ID 0x0010 // ID of network

/*
 new ID system TODO:
 addr 0xA*** - Anchor
 addr 0xB*** - Tag
*/
#define ANCHOR_ID_marker 0xA000
#define TAG_ID_marker    0xB000

#ifdef TAG
  static char whoami[] = "TAG";
  static uint16 my_addr = TAG_ID_marker | COMPILE_ID;
#endif
#ifdef ANCHOR
  static char whoami[] = "ANCHOR";
  static uint16 my_addr = ANCHOR_ID_marker | COMPILE_ID;
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

// #define TRACE_MSG_ON

/* Default antenna delay values for 64 MHz PRF. See NOTE 1 below. */
#define TX_ANT_DLY 16436 ///< только там где мы сами вычисляем TX_TS надо + TX_ANT_DELAY
#define RX_ANT_DLY 16436 ///< только для настройки DWT

static dwt_config_t config =
{
    5,               /* Channel number. */
    DWT_PRF_64M,     /* Pulse repetition frequency. */
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


/*
Формула времени передчаи всего сообщения:
  t = preamblua_len us + sfd_len us + ((19 bit + data_len byte) / data_rate)

для preamblua_len = 128, PRF = 16M => sfd_len = 8, data_len = 127 (макс по стандарту)
6.8Мбит/с
= 288 us
*/

// ==================== PROTOLOCS CONFIG ====================

/* SNIFF mode on/off times.
 * ON time is expressed in multiples of PAC size (with the IC adding 1 PAC automatically). So the ON time of 1 here gives 2 PAC times and, since the
 * configuration (above) specifies DWT_PAC8, we get an ON time of 2x8 symbols, or around 16 �s.
 * OFF time is expressed in multiples of 128/125 �s (~1 �s).
 * These values will lead to a roughly 50% duty-cycle, each ON and OFF phase lasting for about 16 �s. */
#define SNIFF_ON_TIME 1   ///< (x + 1) * PACSIZE (~ 1 us)
#define SNIFF_OFF_TIME 16 ///< ~ 1 us

#define DEFAULT_RX_TIMEOUT_UUS      300

#define INITIATE_PULL_ONE_TIMEOUT_MS 1000


/// инициация со стороны тэга и теперь тэг ждет столько до начала RX чтобы поймать RESP
#define POLL_TX_TO_RESP_RX_DLY_UUS      2000uLL
/// якорь поймал POLL и отправит RESP через
#define POLL_RX_TO_RESP_TX_DLY_UUS      2100uLL
/// тэг ждёт в течении
#define RESP_RX_TIMEOUT_UUS             DEFAULT_RX_TIMEOUT_UUS

/// якорь ждет после RESP чтобы начать RX для FINAL
#define RESP_TX_TO_FINAL_RX_DLY_UUS     1900uLL
/// тэг поймал RESP и отправит FINAL через
#define RESP_RX_TO_FINAL_TX_DLY_UUS     2100uLL
/// якорь ждёт
#define FINAL_RX_TIMEOUT_UUS            500

/// тэг ждёт после FINAL чтобы начать RX для DISTANCE
#define FINAL_TX_TO_DISTANCE_RX_DLY_UUS 2000uLL
/// якорь моймал FINAL и отправит DISTANCE через
#define FINAL_RX_TO_DISTANCE_TX_DLY_UUS 2100uLL
/// тэг ждёт
#define DISTANCE_RX_TIMEOUT_UUS         DEFAULT_RX_TIMEOUT_UUS


// #define PRE_TIMEOUT 8

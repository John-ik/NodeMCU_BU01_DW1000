/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#ifndef COMPILE_ID
  #error "COMPILE_ID not presented. Used for set ID during compilation"
#endif

#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_sleep.h"

#include "mac.h"
#include "deca_leds.h"
#include "messages.h"
#include "event.h"

#include "port.h"
#include "platform_spi.h"

#include "string.h"
#include "stdio.h"

#define DEBUG_UART_TRANSMIT
#include "debug.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* Default antenna delay values for 64 MHz PRF. See NOTE 1 below. */
#define TX_ANT_DLY 16436
#define RX_ANT_DLY 16436
// #define TX_ANT_DLY 0
// #define RX_ANT_DLY 32950
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
char uart_buf[512]; 

static dwt_config_t config =
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
    0 // (2048 + 1 + 64 - 8) /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
};

/* Frames used in the ranging process. See NOTE 2 below. */


static MacMessage pull_one_msg = {
  0x4188, 0, 0, 0, 0, MSG_PULL_ONE, MSG_DATA_EMPTY, 0
};

static MacMessage resp_one_msg = {
  0x4188, 0, 0, 0, 0, MSG_RESP_ONE, MSG_DATA_EMPTY, 0
};


static uint8 tx_buffer[MSG_MAX_LEN];
static uint8 rx_buffer[MSG_MAX_LEN];
static MacMessage msg_buffer;
/*
MAC
MSG:
  0,1: Frame Control
  2:   Number Sequence
  3,4: Dest PAN ID
  5,6: Dest Address
  7,8: Source Address
  9:   My Type

  n-2,n-1: FCS - CRC (not for user. let with zeros)
*/
static uint8 poll_msg[] =  {0x41, 0x88, 0, 0x00, 0xFF, 0xFF, 0xFF, 'M', 'C', MSG_PULL, 0, 0};
static uint8 resp_msg[] =  {0x41, 0x88, 0, 0x00, 0xFF, 0xFF, 0xFF, 'M', 'C', MSG_RESPONSE, 0x02, 0, 0, 0, 0};
static uint8 final_msg[] = {0x41, 0x88, 0, 0x00, 0xFF, 0xFF, 0xFF, 'M', 'C', MSG_FINAL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static uint8 distance_msg[] = {0x41, 0x88, 0, 0x00, 0xFF, 0xFF, 0xFF, 'M', 'C', MSG_DISTANCE, 0, 0,0, 0, 0};

/* Length of the common part of the message (up to and including the function code, see NOTE 2 below). */
#define ALL_MSG_COMMON_LEN 10
/* Index to access some of the fields in the frames involved in the process. */
#define ALL_MSG_SEQ_NUM 2
#define ALL_MSG_TARGET 3
#define ALL_MSG_TYPE    9
#define FINAL_MSG_POLL_TX_TS_IDX 10
#define FINAL_MSG_RESP_RX_TS_IDX 14
#define FINAL_MSG_FINAL_TX_TS_IDX 18
#define FINAL_MSG_TS_LEN 4
#define ANGLE_MSG_IDX 10
#define LOCATION_FLAG_IDX 11
#define LOCATION_INFO_LEN_IDX 12
#define LOCATION_INFO_START_IDX 13
#define ANGLE_MSG_MAX_LEN 30

/* Frame sequence number, incremented after each transmission. */
static uint8 frame_seq_nb = 0;
static uint8 frame_beacon_seq_nm = 0;
static uint8 frame_seq_nb_semaphore = 0;

/* Buffer to store received messages.
 * Its size is adjusted to longest frame that this example code is supposed to handle. */
// #define RX_BUF_LEN 24
// static uint8 rx_buffer[RX_BUF_LEN];

/* Hold copy of status register state here for reference, so reader can examine it at a breakpoint. */
static uint32 status_reg = 0;

/* UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) conversion factor.
 * 1 uus = 512 / 499.2 ? and 1 ? = 499.2 * 128 dtu. */
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
static uint16 request_to_response_delay = 100; 

/* Timestamps of frames transmission/reception.
 * As they are 40-bit wide, we need to define a 64-bit int type to handle them. */
typedef signed long long int64;
typedef unsigned long long uint64;

/* Speed of light in air, in metres per second. */
#ifndef SPEED_OF_LIGHT
#define SPEED_OF_LIGHT 299702547
#endif

static int debug_var;

/*
 new ID system
 addr 0xA*** - Anchor
 addr 0xB*** - Tag
*/
#define MY_PAN_ID 0x0010 // ID of network
uint16 my_addr = COMPILE_ID; // Anchor

// Indexing TAGs and Anchor from 1. 0 is special

// #define TAG
#define TAG_ID 0x0F
#define MASTER_TAG 0x0F
#define MAX_SLAVE_TAG 0x02 
#define SLAVE_TAG_START_INDEX 0x01

#define ANCHOR
#define ANCHOR_MAX_NUM 1
#define ANCHOR_IND 1  // 0 1 2
//#define ANCHOR_IND ANCHOR_NUM

#ifdef TAG
  char IAMIS[] = "TAG";
#endif
#ifdef ANCHOR
  char IAMIS[] = "ANCHOR";
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
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static uint64 get_tx_timestamp_u64(void);
static uint64 get_rx_timestamp_u64(void);
static void final_msg_get_ts(const uint8 *ts_field, uint32 *ts);
static void final_msg_set_ts(uint8 *ts_field, uint64 ts);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void dwt_custom_softReset() {
	uint8_t pmscctrl0[PMSC_CTRL0_LEN];
	dwt_readfromdevice(PMSC_ID, PMSC_CTRL0_OFFSET, PMSC_CTRL0_LEN, pmscctrl0);
	pmscctrl0[0] = 0x01;
	dwt_writetodevice(PMSC_ID, PMSC_CTRL0_OFFSET, PMSC_CTRL0_LEN, pmscctrl0);
	pmscctrl0[3] = 0x00;
	dwt_writetodevice(PMSC_ID, PMSC_CTRL0_OFFSET, PMSC_CTRL0_LEN, pmscctrl0);
	deca_sleep(10);
	pmscctrl0[0] = 0x00;
	pmscctrl0[3] = 0xF0;
	dwt_writetodevice(PMSC_ID, PMSC_CTRL0_OFFSET, PMSC_CTRL0_LEN, pmscctrl0);
}
#define TX_PGDELAY_CH5 0xC5

void configureTXPower(dwt_txconfig_t *config){
    config->PGdly = TX_PGDELAY_CH5;
    config->power = 0x1F1F1F1F;
    dwt_configuretxrf(config);
}

/**
 * @brief get systime in 64-bit 
 *        dwt return systime 40-bit value in 5 bytes
 * @return 40-bit systime in 64-bit
 */
static uint64 get_systime_u64(void){
  uint8 buf[10];
  uint64 time = 0;
  dwt_readsystime(buf);
  for (int8 i = 4; i >= 0; i--){
    time <<= 8;
    time |= buf[i];
  }
  return time;
}

/**
 * @brief softreset receiver-only
 * @details reg 0x36:00 - 7.2.50.1
 */
void softreset_receiver(){
  uint8 buf = 0;

  dwt_readfromdevice(PMSC_ID, 0, 1, &buf);
  buf = (buf & 0x03) | 0x01; // set SYSCLKS to 01
  dwt_writetodevice(PMSC_ID, 0, 1, &buf);

  dwt_readfromdevice(PMSC_ID, 0x3, 1, &buf);
  buf &= 0xEF; // clear only bit 28 to reset only receiver
  dwt_writetodevice(PMSC_ID, 0x3, 1, &buf); 

  buf &= 0x1F; // set only bit 28 to reset only receiver
  dwt_writetodevice(PMSC_ID, 0x3, 1, &buf); 
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn get_tx_timestamp_u64()
 *
 * @brief Get the TX time-stamp in a 64-bit variable.
 *        /!\ This function assumes that length of time-stamps is 40 bits, for both TX and RX!
 *
 * @param  none
 *
 * @return  64-bit value of the read time-stamp.
 */
static uint64 get_tx_timestamp_u64(void)
{
    uint8 ts_tab[5];
    uint64 ts = 0;
    int i;
    dwt_readtxtimestamp(ts_tab);
    for (i = 4; i >= 0; i--)
    {
        ts <<= 8;
        ts |= ts_tab[i];
    }
    return ts;
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn get_rx_timestamp_u64()
 *
 * @brief Get the RX time-stamp in a 64-bit variable.
 *        /!\ This function assumes that length of time-stamps is 40 bits, for both TX and RX!
 *
 * @param  none
 *
 * @return  64-bit value of the read time-stamp.
 */
static uint64 get_rx_timestamp_u64(void)
{
    uint8 ts_tab[5];
    uint64 ts = 0;
    int i;
    dwt_readrxtimestamp(ts_tab);
    for (i = 4; i >= 0; i--)
    {
        ts <<= 8;
        ts |= ts_tab[i];
    }
    return ts;
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn final_msg_get_ts()
 *
 * @brief Read a given timestamp value from the final message. In the timestamp fields of the final message, the least
 *        significant byte is at the lower address.
 *
 * @param  ts_field  pointer on the first byte of the timestamp field to read
 *         ts  timestamp value
 *
 * @return none
 */
static void final_msg_get_ts(const uint8 *ts_field, uint32 *ts)
{
    int i;
    *ts = 0;
    for (i = 0; i < FINAL_MSG_TS_LEN; i++)
    {
        *ts += ts_field[i] << (i * 8);
    }
}
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn final_msg_set_ts()
 *
 * @brief Fill a given timestamp field in the final message with the given value. In the timestamp fields of the final
 *        message, the least significant byte is at the lower address.
 *
 * @param  ts_field  pointer on the first byte of the timestamp field to fill
 *         ts  timestamp value
 *
 * @return none
 */
static void final_msg_set_ts(uint8 *ts_field, uint64 ts)
{
    int i;
    for (i = 0; i < FINAL_MSG_TS_LEN; i++)
    {
        ts_field[i] = (uint8) ts;
        ts >>= 8;
    }
}

/*! @brief 
 @param[in] msg MacMessage
 @param[in] tx_mode pass to `dwt_starttx`
 @return `DWT_SUCCESS` for success, or `DWT_ERROR` for error (e.g. a delayed transmission will fail if the delayed time has passed),
          or `-2` for MSG_ERROR_RX-TX types
 */
int sendtx(MacMessage msg, uint8 tx_mode){
  uint16 frame_len = msg2bytes(msg, tx_buffer);

  // showMsg(uart_buf, msg);
  // DEBUG_transmit_fmt("TXing: %s", uart_buf);

  if (msg.type == MSG_ERROR_RX || msg.type == MSG_ERROR_TX)
    return -2;

  dwt_writetxdata(frame_len, tx_buffer, 0);
  dwt_writetxfctrl(frame_len, 0);

  return debug_var = dwt_starttx(tx_mode);
}

uint16 recieverx(){
  uint16 frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
  if (frame_len <= MSG_MAX_LEN){
    dwt_readrxdata(rx_buffer, frame_len, 0);

    msg_buffer = bytes2msg(rx_buffer, frame_len);
    
    // dwt_readfromdevice(RX_TIME_ID, 0, 14, rx_buffer);
    // char* uart_buf_ptr = uart_buf; 
    // for (size_t i = 0; i < 14; i++){
    //   sprintf(uart_buf_ptr, "%02X", rx_buffer[i]);
    //   uart_buf_ptr += 2;
    // }
    // DEBUG_transmit_fmt("reg 0x15 RX_TIME = %s", uart_buf);
  }
  return frame_len;
}

// ------------------------------ STATE ------------------------------


#define STATE_none     0
#define STATE_Receive  1
#define STATE_Pull_one 2
typedef uint32 State;
static State state = STATE_none;

char* showState(State state){
  switch (state)
  {
  case STATE_none:     return "STATE_none";
  case STATE_Receive:  return "STATE_Receive";
  case STATE_Pull_one: return "STATE_Pull_one";
  default:
    return "! UNDEFINED STATE !";
  }
}

#define STATUS_TIMEOUT(st) st & SYS_STATUS_RXRFTO
#define STATUS_OK(st)      st & SYS_STATUS_RXFCG

/// @brief turn on RX
void toReceive(){
  dwt_rxenable(0);
}

/*!
 * @brief set RX timeout and turn on 
 * input parameters
 * @param time - how long the receiver remains on from the RX enable command
 *               The time parameter used here is in 1.0256 us (512/499.2MHz) units
 *               If set to 0 the timeout is disabled.
 */
void toReceiveInTime(uint16 time){
  dwt_setrxtimeout(time);
  toReceive();
}

void step(MsgEvent event){
  uint64 pull_rx_ts, resp_tx_time;
  
  if (event == EVENT_none) return;

  switch(state){
    case STATE_Receive:
      switch(event){
        case EVENT_initiate_pull_one: // in STATE_Receive
          led_signal(2);

          pull_one_msg.seq_num = frame_seq_nb++;
          pull_one_msg.dest_pan  = MY_PAN_ID;
          pull_one_msg.dest_addr = 0xFFFF;
          pull_one_msg.src_addr  = my_addr;
          
          state = STATE_Pull_one; // mutate state

          sendtx(pull_one_msg, DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);
          return;

        case MSG_PULL_ONE: // in STATE_Receive
          led_signal(3);
          pull_rx_ts = dwt_readrxtimestamphi32() << 8;
          resp_tx_time = (pull_rx_ts + (request_to_response_delay * UUS_TO_DWT_TIME));
          dwt_setdelayedtrxtime((uint32)(resp_tx_time >> 8)); 
          //? set rx timreout and rxaftertxdelay

          // uint64 resp_tx_ts = (((uint64)(resp_tx_time & 0xFFFFFFFE)) << 8) + TX_ANT_DLY;
          resp_one_msg.seq_num   = msg_buffer.seq_num;
          resp_one_msg.dest_addr = msg_buffer.src_addr;
          resp_one_msg.dest_pan  = MY_PAN_ID;
          resp_one_msg.src_addr  = my_addr;
          resp_one_msg.data.resp_one.pull_rx_ts = (uint32)pull_rx_ts;
          resp_one_msg.data.resp_one.resp_tx_ts = (uint32)resp_tx_time; // + TX_ANT_DLY

          int err = sendtx(resp_one_msg, DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED);
          if (err == DWT_ERROR) // correct delay
            request_to_response_delay += 100;
          dwt_rxenable(0);
          // DEBUG_transmit_fmt("debug: %X00, %X00, %X00", (uint32)pull_rx_ts >> 8, (uint32)resp_tx_time >> 8, (uint32)debug_var >> 8);

          return;

        default: // in STATE_Receive
          toReceiveInTime(0);
          return;
      }
      return;

    case STATE_Pull_one:
      switch(event){
        case MSG_RESP_ONE: // in STATE_Pull_one
          DEBUG_transmit_str("resp_one");
          toReceive();
          state = STATE_Receive; // state mutate
          return;

        default: // in STATE_Pull_one
          DEBUG_transmit_str("pull_one: default");
          toReceive();
          state = STATE_Receive; // state mutate
          return;
      }
      return;

    default:
      return;
  }
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ STATE ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(100);
  // reset_DW1000();
  // dwt_softreset();
  dwt_custom_softReset();
  HAL_Delay(100);
  
  
  DEBUG_transmit_str("starting");
  DEBUG_transmit_fmt("0x%X", dwt_readdevid());
  spi_low_speed();
  DEBUG_transmit_fmt("0x%X", dwt_readdevid());
  while (dwt_initialise(DWT_LOADUCODE) == DWT_ERROR){
    DEBUG_transmit_str("ERRORO");
    reset_DW1000();
  }
  spi_full_speed();
  HAL_Delay(100);

  /* Configure DW1000. See NOTE 6 below. */
  if(dwt_configure(&config) == DWT_ERROR){
    DEBUG_transmit_str("!!! CONFIGURE ERROR !!!");
  }

  // recommended from Software_API_Guide
  // dwt_txconfig_t txconfig = {
  //   .PGdly = 0xC0,
  //   .power = 0x0E082848
  // };
  // configureTXPower(&txconfig);

  /* Apply default antenna delay value. See NOTE 1 below. */
  dwt_setrxantennadelay(RX_ANT_DLY);
  dwt_settxantennadelay(TX_ANT_DLY);

  DEBUG_transmit_str("INITED");


  // // set TX/RX leds
  // {
  //   dwt_on_deboundsclock();
  //   dwt_blinking_enable();
  //   dwt_gpio_mode(GPIO_RXOKLED, MODE_OUTPUT);
  //   dwt_gpio_mode(GPIO_RXLED, MODE_OUTPUT);
  //   dwt_gpio_mode(GPIO_TXLED, MODE_OUTPUT);
  //   deca_sleep(10);
  //   // blink all leds
  //   dwt_blink_leds();
  // }

  // PRINT WHO IS WHO
  DEBUG_transmit_fmt("I am is a %s", IAMIS);

  // Confifure filtering
  // dwt_enableframefilter(DWT_FF_DATA_EN);
  uint32 cfg = dwt_read32bitreg(SYS_CFG_ID);
  DEBUG_transmit_fmt("Sys_cfg = 0x%X; Status = 0x%X", cfg, dwt_read32bitreg(SYS_STATUS_ID));

  dwt_configeventcounters(1); // enalbe counters for diagnostics


  // INITIAL STATE
  state = STATE_Receive;

  led_signal(0);

  dwt_setrxtimeout(0);
  dwt_rxenable(0); // start RX
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    led_signal(0);

    
    

    uint8 was_timer = 0;
    uint32 timer_timeout = 1000;
    { // wait for RX or user timeout
      uint32 timer = HAL_GetTick();
      while (
        !((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR))
        #ifdef TAG
          && (HAL_GetTick() - timer < timer_timeout)
        #endif
          )
      { }; // Wait Recieve Good or Error
      #ifdef TAG
        was_timer = HAL_GetTick() - timer >= timer_timeout;
        if (was_timer)
          dwt_forcetrxoff(); // shutdown TX/RX
      #endif
    }
    if (status_reg & SYS_STATUS_CLKPLL_LL){
      DEBUG_transmit_str("!!! Clock PLL Losing Lock. !!!");
    }
    if (status_reg & SYS_STATUS_RFPLL_LL){
      DEBUG_transmit_str("!!! RF PLL Losing Lock. !!!");
    }


    led_signal(1);
    MyEvents event = 0;
    if (STATUS_OK(status_reg)){
      // clear good bits
      dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);

      recieverx(); // place msg in msg_buf
      
      event = toMsgEvent(status_reg, msg_buffer.type, 0);
      
      showMsg(uart_buf, msg_buffer);
      DEBUG_transmit_fmt("%s at %u", uart_buf, dwt_readrxtimestamplo32());
    }else if (was_timer){
      event = EVENT_rxtimeout;

      #ifdef TAG
        static uint32 timer_pull_one = 0; 
        if (HAL_GetTick() - timer_pull_one > 5000){
          timer_pull_one = HAL_GetTick();
          event = EVENT_initiate_pull_one;
        }
      #endif

      // dump Digital Diagnostics Interface
      {
          dwt_deviceentcnts_t cntrs;
          dwt_readeventcounters(&cntrs);
          DEBUG_transmit_fmt("DIAG:\n"
                            "\tPHR er = %u"
                            "\tRSD er = %u"
                            "\tcrc good = %u"
                            "\tcrc bad = %u"
                            "\tframe filter reject = %u"
                            "\toverrun = %u"
                            "\tsfd timeout = %u"
                            "\tpreamble timeout = %u"
                            "\tRX timeout = %u"
                            "\tTX sent = %u"
                            "\tperiod/2 (big TX delay)= %u"
                            "\tshort TX delay = %u\n"
                            "\treq2resp = %u", 
            cntrs.PHE, cntrs.RSL, cntrs.CRCG, cntrs.CRCB, cntrs.ARFE, cntrs.OVER, cntrs.SFDTO, cntrs.PTO, cntrs.RTO, cntrs.TXF, cntrs.HPW, cntrs.TXW,
            request_to_response_delay
          );
      }
    }else{
      // clear errors
      dwt_write32bitreg(SYS_STATE_ID, SYS_STATUS_ALL_RX_ERR);
      // reset receiver for correctly calc timestamp in future
      // softreset_receiver();
      dwt_rxreset();

      // if (HAL_GetTick() - timer > 5000){
      //   event = toMsgEvent(0, 0, EVENT_initiate_pull_one);
      //   timer = HAL_GetTick();
      // }else{
      event = toMsgEvent(status_reg, 0, 0);
      // }
    }
    DEBUG_transmit_fmt("BEFORE STEP: event = %s; state = %s; Status_reg = %u", showEvent(event), showState(state), status_reg);
    step(event);
    DEBUG_transmit_fmt("AFTER STEP: state = %s; debug_var = %d; delay = %u", showState(state), debug_var, request_to_response_delay);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

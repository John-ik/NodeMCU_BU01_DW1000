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
#include "i2c.h"
#include "spi.h"
#include "usart.h"
#include "usb_device.h"
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
#include "deca_funcs.h"
#include "event.h"

#include "port.h"
#include "platform_spi.h"

#include "string.h"
#include "stdio.h"

#define DEBUG_UART_TRANSMIT
// #define DEBUG_USB_TRANSMIT
#include "debug.h"

#include "config.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
char uart_buf[512]; 
char DEBUG_uart_buf[630]; 


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
uint8 poll_msg[] =  {0x41, 0x88, 0, 0x00, 0xFF, 0xFF, 0xFF, 'M', 'C', MSG_PULL, 0, 0};
uint8 resp_msg[] =  {0x41, 0x88, 0, 0x00, 0xFF, 0xFF, 0xFF, 'M', 'C', MSG_RESPONSE, 0x02, 0, 0, 0, 0};
uint8 final_msg[] = {0x41, 0x88, 0, 0x00, 0xFF, 0xFF, 0xFF, 'M', 'C', MSG_FINAL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8 distance_msg[] = {0x41, 0x88, 0, 0x00, 0xFF, 0xFF, 0xFF, 'M', 'C', MSG_DISTANCE, 0, 0,0, 0, 0};


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

static uint16 request_to_response_delay = 10; 

static uint8 was_sended = 0; 

static int debug_var;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
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

  led_signal(msg.seq_num & 7);

  dwt_writetxdata(frame_len, tx_buffer, 0);
  dwt_writetxfctrl(frame_len, 0);

  return debug_var = dwt_starttx(tx_mode);
}

/**
 * @brief get MacMessage and place in `msg_buffer`
 */
uint16 recieverx(){
  uint16 frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
  if (frame_len <= MSG_MAX_LEN){
    dwt_readrxdata(rx_buffer, frame_len, 0);

    msg_buffer = bytes2msg(rx_buffer, frame_len);
    led_signal(msg_buffer.seq_num & 7);
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

/// @brief Use before Transmit and config transmit
void toIdle(){
  dwt_forcetrxoff();
}

void step(MsgEvent event){
  uint64 pull_rx_ts, resp_tx_time;
  uint64 req_tx_ts, ans_rx_ts, ans_tx_ts, req_rx_ts;

  switch(state){
    case STATE_Receive:
      switch(event){
        case EVENT_initiate_pull_one: // in STATE_Receive
          // led_signal(2);
          toIdle();
          dwt_setrxtimeout(PULL_ONE_TIMEOUT_US);

          pull_one_msg.seq_num = frame_seq_nb++;
          pull_one_msg.dest_pan  = MY_PAN_ID;
          pull_one_msg.dest_addr = 0xFFFF;
          pull_one_msg.src_addr  = my_addr;
          
          state = STATE_Pull_one; // mutate state

          sendtx(pull_one_msg, DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);
          return;

        case MSG_PULL_ONE: // in STATE_Receive
          // led_signal(3);
          toIdle();

          pull_rx_ts = get_rx_ts();
          resp_tx_time = (pull_rx_ts + (2800 * UUS_TO_DWT_TIME));
          dwt_setdelayedtrxtime((uint32) (resp_tx_time) >> 8); 
          //? set rx timreout and rxaftertxdelay

          // uint64 resp_tx_ts = (((uint64)(resp_tx_time & 0xFFFFFFFE)) << 8) + TX_ANT_DLY;
          resp_one_msg.seq_num   = msg_buffer.seq_num;
          resp_one_msg.dest_addr = msg_buffer.src_addr;
          resp_one_msg.dest_pan  = MY_PAN_ID;
          resp_one_msg.src_addr  = my_addr;
          resp_one_msg.data.resp_one.pull_rx_ts = pull_rx_ts;
          resp_one_msg.data.resp_one.resp_tx_ts = resp_tx_time; // + TX_ANT_DLY

          int err = sendtx(resp_one_msg, DWT_START_TX_DELAYED);

          showMsg(uart_buf, resp_one_msg);
          DEBUG_transmit_fmt("sended: %s", uart_buf);

          return;

        default: // in STATE_Receive
          toReceiveInTime(0);
          return;
      }
      return; // after switch(event) <-- STATE_Receive

    case STATE_Pull_one:
      switch(event){
        case MSG_RESP_ONE: // in STATE_Pull_one
          req_tx_ts = get_tx_ts();
          ans_rx_ts = get_rx_ts();
          ans_tx_ts = msg_buffer.data.resp_one.resp_tx_ts;
          req_rx_ts = msg_buffer.data.resp_one.pull_rx_ts;

          float time = (float) ((ans_rx_ts - req_tx_ts) - (ans_tx_ts - req_rx_ts)) / 2;
          float dist = time * SPEED_OF_LIGHT / (128 * 499.2 * 1000000);
          DEBUG_transmit_fmt("frame_seq_nb = %u\nreq_tx = %f, req_rx = %f, ans_tx = %f, ans_rx = %f, dist: %f m",
                  frame_seq_nb - 1,    
                  (float) req_tx_ts, (float) req_rx_ts, (float) ans_tx_ts, (float) ans_rx_ts, dist);

          toReceive();
          state = STATE_Receive; // state mutate
          return;

        default: // in STATE_Pull_one
          DEBUG_transmit_str("pull_one: default");
          toReceive();
          state = STATE_Receive; // state mutate
          return;
      }
      return; // <-- after switch(event) STATE_Pull_one

    default:
      return;
  }
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ STATE ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// ============================== HANDLERS ==============================

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  if (GPIO_Pin == IRQ_Pin){
    dwt_irq();
  }
}

MyEvents event = EVENT_none;

// void handler_rxok(uint32 status){
  
// }
// _dwt_handler_rxok = &handler_rxok;

void handler_send(uint32 status){
  UNUSED(status);
  was_sended = 1;

  // TXFRS automatical clear on next transmit
}

static uint8 pll_err_counter = 0;
static uint32 last_pll_err = 0;
void handler_pll_error(uint32 status){
  pll_err_counter++;
  dwt_reset_status(DWT_IRQ_PLL_ERROR);
  last_pll_err = status & DWT_IRQ_PLL_ERROR;
  event = EVENT_pll_error;
}

void handler_rxok(uint32 status){
  UNUSED(status);
  recieverx();
  event = msg_buffer.type;

  // RXFCG automatical clear on next receive
}

static uint32 last_rxfailed_status = 0;
void handler_rxfailed(uint32 status){
  dwt_reset_status(status & DWT_IRQ_RXFAILED);
  last_rxfailed_status = status & DWT_IRQ_RXFAILED;
}

void handler_rxtimeout(uint32 status){
  UNUSED(status);
  event = EVENT_rxtimeout;
}

/// assign dwt_handlers
void init_irq(){
  _dwt_handler_send = &handler_send;
  _dwt_handler_pll_error = &handler_pll_error;
  _dwt_handler_rxok = &handler_rxok;
  _dwt_handler_rxfailed = &handler_rxfailed;
  _dwt_handler_rxtimeout = &handler_rxtimeout;

  dwt_setinterrupt(
    DWT_IRQ_SEND |
    DWT_IRQ_PLL_ERROR |
    DWT_IRQ_RXOK |
    DWT_IRQ_RXFAILED |
    DWT_IRQ_RXTIMEOUT
    , 1
  );
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ HANDLERS ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
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
  MX_I2C1_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  spi_low_speed();
  // HAL_Delay(100);
  // reset_DW1000();
  // dwt_softreset();
  dwt_custom_softReset();
  // HAL_Delay(100);
  
  
  DEBUG_transmit_str("starting");
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

  dwt_reset_status(SYS_STATUS_SLP2INIT | SYS_STATUS_CPLOCK);
  DEBUG_transmit_str("INITED");


  // set TX/RX leds
  {
    // dwt_on_deboundsclock();
    // dwt_blinking_enable();
    // dwt_gpio_mode(GPIO_RXLED, MODE_OUTPUT);
    // dwt_gpio_mode(GPIO_TXLED, MODE_OUTPUT);
    // deca_sleep(10);
    
    // blink all leds
    // dwt_blink_leds();
    
    dwt_gpio_mode(GPIO_RXOKLED, MODE_OUTPUT); // dwt_setleds enable only TX/RX
    dwt_setleds(1);
  }

  // PRINT WHO IS WHO
  DEBUG_transmit_fmt("I am a %s", whoami);

  // Confifure filtering
  // dwt_enableframefilter(DWT_FF_DATA_EN);
  
  dwt_configeventcounters(1); // enalbe counters for diagnostics
  
  // enable IRQ
  init_irq();
  
  uint32 cfg = dwt_read32bitreg(SYS_CFG_ID);
  DEBUG_transmit_fmt("Sys_cfg = 0x%X; Status = 0x%X", cfg, dwt_get_status());


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
    if (event){
      DEBUG_transmit_fmt("BEFORE: event = %s; state = %s; status = 0x%X; seq = %u",
        showEvent(event), showState(state), status_reg, frame_seq_nb
      );
    }

    if (EVENT_is(event, EVENTs_dwt)){ // Aka server event
      // EVENT_rxtimeout processing in step

      if (event == EVENT_pll_error){
        if (last_pll_err & SYS_STATUS_CLKPLL_LL){
          DEBUG_transmit_fmt("!!! Clock PLL Losing Lock. №%u (common counter with RF_PLL_LL) !!!", pll_err_counter);
        }
        if (last_pll_err & SYS_STATUS_RFPLL_LL){
          DEBUG_transmit_fmt("!!! RF PLL Losing Lock. №%u (common counter with CPLL_LL) !!!", pll_err_counter);
        }
      }
    }

    if (EVENT_is(event, EVENTs_host)){ // Aka client event

    }

    if (event && (EVENT_is(event, EVENTs_custom) || EVENT_is(event, EVENTs_msg) || event == EVENT_rxtimeout)){ // All for msg protocols
      step(event);

      DEBUG_transmit_fmt("AFTER STEP: state = %s; debug_var = %d; status = 0x%X", showState(state), debug_var, dwt_read32bitreg(SYS_STATUS_ID));
    }

    event = EVENT_none;

    #ifdef TAG
      static uint32 timer_pull_one = 0; 
      if (HAL_GetTick() - timer_pull_one > 5000){
        timer_pull_one = HAL_GetTick();
        event = EVENT_initiate_pull_one;
      }
    #endif

    #ifdef DEBUG_DWT_DIAG
      static uint32 timer_diag = 0;
      if (HAL_GetTick() - timer_diag > DEBUG_DWT_DIAG_TIMEOUT){
        timer_diag = HAL_GetTick();
        dwt_showDiag(uart_buf);
        Transmit(uart_buf);
        Transmit("\n");
      }
    #endif
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV2;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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

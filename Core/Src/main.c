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
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#ifndef COMPILE_ID
  #error "COMPILE_ID not presented. Used for set ID during compilation"
#endif

#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_sleep.h"

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
#define UART_BUF_len 512
char uart_buf[UART_BUF_len]; 
char DEBUG_uart_buf[630];
uint32 DEBUG_sys_ts;

/* Frame sequence number, incremented after each transmission. */
static uint8 frame_seq_nb = 0;
static uint8 frame_beacon_seq_nm = 0;
static uint8 frame_seq_nb_semaphore = 0;

/* Buffer to store received messages.
 * Its size is adjusted to longest frame that this example code is supposed to handle. */
static uint8 rx_buffer[MSG_MAX_LEN];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/

/* USER CODE BEGIN 0 */

void trace_msg(uint8 msg[]){
  char *buf = uart_buf;
  size_t buf_size = UART_BUF_len;
  uint8 msg_type = MSG_TYPE(msg);
  uint8 msg_len = msgGetLen(msg_type);

  uint64_t tx_ts = get_tx_ts();
  uint64_t rx_ts = get_rx_ts();
  int printed = 0;
  for(size_t i = 0; i < msg_len; i++){
        printed = snprintf(buf, buf_size, "%02X", msg[i]);
        buf += printed;
        buf_size -= printed;
  }
  DEBUG_transmit_fmt("TRACE: 0x%02X%08lX 0x%02X%08lX %s", 
    (uint8)(tx_ts >> 32),  (uint32) tx_ts,
    (uint8)(rx_ts >> 32),  (uint32) rx_ts,
    uart_buf
  );
}
#ifdef TRACE_MSG_ON
  #define TRACE_MSG(msg) trace_msg(msg)
#else
  #define TRACE_MSG(msg) do {} while(0)
#endif

#define RANGING_ON 1
#define RANGING_OFF 0
/*! @brief 
 @param[in] msg массив байтов представляющий собой сообщение
 @param[in] tx_mode pass to `dwt_starttx`
 @param[in] ranging pass to `dwt_writetxfctrl` (true for ranging)
 @return `DWT_SUCCESS` for success, or `DWT_ERROR` for error (e.g. a delayed transmission will fail if the delayed time has passed)
 */
int sendtx(uint8 msg[], uint8 msg_len, uint8 tx_mode, const int ranging){
  led_signal(MSG_SEQNUM(msg) & 7);

  // showMsg(uart_buf, UART_BUF_len, msg);
  // DEBUG_transmit_str(uart_buf);

  dwt_writetxdata(msg_len, msg, 0);
  dwt_writetxfctrl(msg_len, 0, ranging);

  return dwt_starttx(tx_mode);
}

/**
 * @brief получает сообщений и кладет в `rx_buffer`
 */
uint16 recieverx(){
  // Transmit("receiverx\n");
  uint16 frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
  if (frame_len <= MSG_MAX_LEN){
    dwt_readrxdata(rx_buffer, frame_len, 0);
    led_signal(MSG_SEQNUM(rx_buffer) & 7);
    // showMsg(uart_buf, UART_BUF_len, rx_buffer);
    // DEBUG_transmit_str(uart_buf);
  }
  return frame_len;
}

// ============================== HANDLERS ==============================

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  if (GPIO_Pin == IRQ_Pin){
    dwt_irq();
  }
}

static uint8 flag_send = 0;
void handler_send(uint32 status){
  UNUSED(status);
  flag_send = 1;
  // Transmit("send\n");
}

static uint8 pll_err_counter = 0;
static uint32 flag_pll_err = 0; // and save which pll issue
void handler_pll_error(uint32 status){
  pll_err_counter++;
  flag_pll_err = status & DWT_IRQ_PLL_ERROR;
}

static uint8 flag_rxok = 0;
void handler_rxok(uint32 status){
  UNUSED(status);
  recieverx();
  flag_rxok = 1;
}

static uint32 flag_rxfailed_status = 0;
void handler_rxfailed(uint32 status){
  flag_rxfailed_status = status & DWT_IRQ_RXFAILED;
}

static uint8 flag_rxtimeout = 0;
void handler_rxtimeout(uint32 status){
  UNUSED(status);
  flag_rxtimeout = 1;
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
// ------------------------------ STATE ------------------------------

typedef enum {
  STATE_none     = 0,
  STATE_Receive  = 1,
  STATE_Pull_one = 2
} State;
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
  dwt_setrxtimeout(0);
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
  dwt_rxenable(0);
}

/// @brief Use before Transmit and config transmit
void toIdle(){
  dwt_forcetrxoff();
}

void step(MsgEvent event){
  uint64_t sys_ts = 0, rx_ts = 0;
  uint64_t pull_rx_ts = 0, resp_tx_ts = 0;
  uint64_t req_tx_ts = 0, ans_rx_ts = 0, ans_tx_ts = 0, req_rx_ts = 0;

  switch(state){
    case STATE_Receive:
      switch(event){
        case EVENT_initiate_pull_one: // in STATE_Receive
          // led_signal(2);
          toIdle();
          dwt_setrxaftertxdelay(POLL_TX_TO_RESP_RX_DLY_UUS);
          dwt_setrxtimeout(RESP_RX_TIMEOUT_UUS);

          MSG_SEQNUM(msg_pull_one) = frame_seq_nb++;
          MSG_PAN_ID(msg_pull_one) = MY_PAN_ID;
          MSG_DEST_ID(msg_pull_one) = 0xFFFF;
          MSG_SRC_ID(msg_pull_one)  = my_addr;
          
          state = STATE_Pull_one; // mutate state

          sendtx(msg_pull_one, MSG_PULL_ONE_len, DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED, RANGING_ON);
          // while( ! flag_send){} // ждём завершения отправки
          TRACE_MSG(msg_pull_one);
          return;

        case EVENT_msg_PULL_ONE: // in STATE_Receive
          // led_signal(3);
          toIdle();

          pull_rx_ts = get_rx_ts();
          // static uint64_t pull_one4resp_delay = 1;
          resp_tx_ts = (pull_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME));
          dwt_setdelayedtrxtime((uint32) (resp_tx_ts >> 8) );
          //? set rx timreout and rxaftertxdelay

          resp_tx_ts = (((uint64_t)(resp_tx_ts & 0xFFFFFFFE00))) + TX_ANT_DLY;
          MSG_SEQNUM(msg_resp_one)  = MSG_SEQNUM(rx_buffer);
          MSG_PAN_ID(msg_resp_one)  = MY_PAN_ID;
          MSG_DEST_ID(msg_resp_one) = MSG_SRC_ID(rx_buffer);
          MSG_SRC_ID(msg_resp_one)  = my_addr;
          MSG_RESP_ONE_pull_rx_ts_set(msg_resp_one, &pull_rx_ts);
          // resp_tx_ts += TX_ANT_DLY;
          MSG_RESP_ONE_resp_tx_ts_set(msg_resp_one, &resp_tx_ts);

          int err = sendtx(msg_resp_one, MSG_RESP_ONE_len, DWT_START_TX_DELAYED, RANGING_ON);
          if (err){
            toReceive();
            return;
          }
          
          // while( ! flag_send){} // ждём завершения отправкиы
          TRACE_MSG(msg_resp_one);
          toReceive();
          return;

        default: // in STATE_Receive
          trace_msg(rx_buffer);
          toReceive();
          return;
      }
      return; // after switch(event) <-- STATE_Receive

    case STATE_Pull_one:
      switch(event){
        case EVENT_msg_RESP_ONE: // in STATE_Pull_one
          req_tx_ts = get_tx_ts();
          ans_rx_ts = get_rx_ts();
          MSG_RESP_ONE_resp_tx_ts_get(rx_buffer, &ans_tx_ts);
          MSG_RESP_ONE_pull_rx_ts_get(rx_buffer, &req_rx_ts);

          float time = (float) ((ans_rx_ts - req_tx_ts) - (ans_tx_ts - req_rx_ts)) / 2;
          float dist = time * SPEED_OF_LIGHT / (128 * 499.2 * 1000000);
          DEBUG_transmit_fmt("frame_seq_nb = %u\nreq_tx = %f, req_rx = %f, ans_tx = %f, ans_rx = %f, dist: %f m",
                  frame_seq_nb - 1,    
                  (float) req_tx_ts, (float) req_rx_ts, (float) ans_tx_ts, (float) ans_rx_ts, dist);

          TRACE_MSG(rx_buffer);

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
/* USER CODE END 0 */
void SystemClock_Config(void);

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
  spi_low_speed();
  // HAL_Delay(100);
  // reset_DW1000();
  // dwt_softreset();
  dwt_custom_softReset();
  // HAL_Delay(100);
  
  
  // DEBUG_transmit_str("starting");
  // DEBUG_transmit_fmt("0x%lX", dwt_readdevid());
  while (dwt_initialise(DWT_LOADUCODE) == DWT_ERROR){
    // DEBUG_transmit_str("ERRORO");
    reset_DW1000();
  }
  spi_full_speed();
  HAL_Delay(100);

  /* Configure DW1000. See NOTE 6 below. */
  dwt_configure(&config);

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
  DEBUG_transmit_fmt("Sys_cfg = 0x%lX; Status = 0x%lX", cfg, dwt_get_status());


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

    MyEvents event = EVENT_none;
    static MyEvents saved_event = EVENT_none;

    if (saved_event == EVENT_none && state <= STATE_Receive){
      #ifdef TAG
        static uint32 timer_pull_one = 0; 
        if (HAL_GetTick() - timer_pull_one > INITIATE_PULL_ONE_TIMEOUT_MS){
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
    
    // flag handlers and saved event
    if (flag_pll_err){
      saved_event = event;
      event = EVENT_pll_error;
      flag_pll_err = 0;
    }else if(flag_rxfailed_status){
      //TODO: Восстановаление  приема после ошибки
      saved_event = event;
      event = EVENT_rxfail;
      flag_rxfailed_status = 0;
    }else if(flag_rxok){
      saved_event = event;
      event = MSG_TYPE_2_EVENT(MSG_TYPE(rx_buffer));
      flag_rxok = 0;
    }else if(flag_rxtimeout){
      saved_event = event;
      event = EVENT_rxtimeout;
      flag_rxtimeout = 0;
    }else if(flag_send){
      flag_send = 0;
    }else if(saved_event){
      event = saved_event;
      saved_event = EVENT_none;
    }

    { // Aka server event
      // EVENT_rxtimeout processing in step

      if (event == EVENT_pll_error){
        if (flag_pll_err & SYS_STATUS_CLKPLL_LL){
          DEBUG_transmit_fmt("!!! Clock PLL Losing Lock. №%u (common counter with RF_PLL_LL) !!!", pll_err_counter);
        }
        if (flag_pll_err & SYS_STATUS_RFPLL_LL){
          DEBUG_transmit_fmt("!!! RF PLL Losing Lock. №%u (common counter with CPLL_LL) !!!", pll_err_counter);
        }
      }

      if (event == EVENT_rxfail){
        dwt_rxenable(0); // start RX
        //TODO: отправка пакета об ошибке,
        state = STATE_Receive;
      }
    }

    { // Aka client event

    }

    // All for msg protocols
    if (EVENT_is(event, EVENTs_custom) || EVENT_is(event, EVENTs_msg) || (event == EVENT_rxtimeout)){
      step(event);
    }
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

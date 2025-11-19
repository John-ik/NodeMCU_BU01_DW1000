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
#define UART_BUF_len 80
char uart_buf[UART_BUF_len];
char DEBUG_uart_buf[128];
uint32 DEBUG_sys_ts;

/* Frame sequence number, incremented after each transmission. */
static uint8 frame_seq_nb = 0;
static uint8 frame_beacon_seq_nm = 0;
static uint8 frame_seq_nb_semaphore = 0;

/// @brief буфер для смс. Самое длинное по стандарту MAC
static uint8_t rx_buffer_raw[127+5];
static uint8_t *rx_buffer = &rx_buffer_raw[5]; // магия с перекрыванием памяти
/// @brief размер принятого смс
static uint16_t rx_len;

static const dwt_cb_data_t *cb_data_p;

static uint64_t *rx_ts_sniffer = (uint64_t*) &rx_buffer_raw[0]; // магия с пересечением памяти

static uint16_t targets_len = 3;
static uint16_t target_indx = 0;
static uint16_t targets[3] = {ANCHOR_ID_marker | 54, ANCHOR_ID_marker | 55, 0xFFFF};

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

  int64_t tx_ts = get_tx_ts();
  int64_t rx_ts = get_rx_ts();
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
#ifdef TAG
  led_signal(MSG_TYPE(msg) & 7);
#endif

  dwt_writetxdata(msg_len, msg, 0);
  dwt_writetxfctrl(msg_len, 0, ranging);

  return dwt_starttx(tx_mode);
}

/// @brief Use before Transmit and config TX/RX
void toIdle(){
  dwt_forcetrxoff();
}

/// @brief Use after RX error or other UB
void toIdleFromErr(){
  dwt_forcetrxoff();
  dwt_rxreset();
}

// ============================== HANDLERS ==============================

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  if (GPIO_Pin == IRQ_Pin){
    /* 
      main call-back for processing of DW1000 IRQ
      it re-enters the IRQ routing and processes all events.
      After processing of all events, DW1000 will clear the IRQ line.
    */
    while(HAL_GPIO_ReadPin(IRQ_GPIO_Port, IRQ_Pin) != 0){
        dwt_isr();
    } //while DW1000 IRQ line active
  }
}

static uint8 flag_send = 0;
void handler_send(const dwt_cb_data_t * data){
  cb_data_p = data;
  flag_send = 1;
}

static uint8 flag_rxok = 0;
void handler_rxok(const dwt_cb_data_t * data){
  cb_data_p = data;
  flag_rxok = 1;
}

void handler_rxfailed(const dwt_cb_data_t * data){
  UNUSED(data);
  dwt_rxenable(DWT_START_RX_IMMEDIATE);
}

static uint8 flag_rxtimeout = 0;
void handler_rxtimeout(const dwt_cb_data_t * data){
  UNUSED(data);
  flag_rxtimeout = 1;
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ HANDLERS ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// ------------------------------ STATE ------------------------------

typedef enum {
  STATE_none     = 0,
  STATE_Receive,
  STATE_wait_RESP,
  STATE_wait_RESP_3,
  STATE_wait_FINAL,
  STATE_wait_DIST,

  STATE_Sniffer  = 0xf0
} State;
static State state = STATE_none;

static uint16 frame_filter = DWT_FF_DATA_EN | DWT_FF_BEACON_EN;

char* showState(State state){
  switch (state)
  {
  case STATE_none:          return "STATE_none";
  case STATE_Receive:       return "STATE_Receive";
  case STATE_wait_RESP:     return "STATE_wait_RESP";
  case STATE_wait_RESP_3:   return "STATE_wait_RESP_3";
  case STATE_wait_FINAL:    return "STATE_wait_FINAL";
  case STATE_wait_DIST:     return "STATE_wait_DIST";
  default:
    return "! UNDEFINED STATE !";
  }
}

#define STATUS_TIMEOUT(st) st & SYS_STATUS_RXRFTO
#define STATUS_OK(st)      st & SYS_STATUS_RXFCG

static int64_t  pull_tx_ts,  pull_rx_ts,
                resp_tx_ts,  resp_rx_ts,
                final_tx_ts, final_rx_ts;

static float dist;
static uint16_t src_addres = 0xFFFF;

void step(MsgEvent event){
  switch(state){
    case STATE_Receive:
      switch(event){
        case EVENT_initiate_pull: // in STATE_Receive
        case EVENT_initiate_pull_3:

          toIdle();
          dwt_setrxaftertxdelay(POLL_TX_TO_RESP_RX_DLY_UUS);
          dwt_setrxtimeout(RESP_RX_TIMEOUT_UUS);

          MSG_SEQNUM(msg_pull)  = ++frame_seq_nb;
          MSG_PAN_ID(msg_pull)  = MY_PAN_ID;
          MSG_DEST_ID(msg_pull) = targets[target_indx];
          MSG_SRC_ID(msg_pull)  = my_addr;
          MSG_TYPE(msg_pull)    = (uint8_t) event; // либо PULL либо PULL_3

          if (event == EVENT_initiate_pull)
            state = STATE_wait_RESP; // mutate state
          else
            state = STATE_wait_RESP_3; // mutatue state

          sendtx(msg_pull, MSG_PULL_len, DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED, RANGING_ON);
          TRACE_MSG(msg_pull);
          return;

        case EVENT_initiate_sniffer: // in STATE_Receive
          toIdle();
          state = STATE_Sniffer;
          dwt_setrxtimeout(0);
          dwt_setsniffmode(1, SNIFF_ON_TIME, SNIFF_OFF_TIME); // настройка режима сниффера
          dwt_enableframefilter(DWT_FF_NOTYPE_EN); // вырубаем фильтрацию, т.к. слушаем всё
          dwt_setdblrxbuffmode(1); // вкл двойной буфер
          dwt_rxenable(DWT_START_RX_IMMEDIATE);
          return;

        case EVENT_msg_PULL_3: // якорь поймал
        case EVENT_msg_PULL: // якорь моймал
          toIdle();

          src_addres = MSG_SRC_ID(rx_buffer);

          pull_rx_ts = get_rx_ts();
          resp_tx_ts = (pull_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME));
          dwt_setdelayedtrxtime((uint32) (resp_tx_ts >> 8) );
          //? set rx timreout and rxaftertxdelay

          resp_tx_ts = (((int64_t)(resp_tx_ts & 0xFFFFFFFE00))) + TX_ANT_DLY;
          MSG_SEQNUM(msg_resp)  = MSG_SEQNUM(rx_buffer);
          MSG_PAN_ID(msg_resp)  = MY_PAN_ID;
          MSG_DEST_ID(msg_resp) = src_addres;
          MSG_SRC_ID(msg_resp)  = my_addr;
          MSG_RESP_pull_rx_ts_set(msg_resp, &pull_rx_ts);
          MSG_RESP_resp_tx_ts_set(msg_resp, &resp_tx_ts);

          if (event == EVENT_msg_PULL_3){
            MSG_TYPE(msg_resp) = MSG_RESP_3;
            dwt_setrxtimeout(FINAL_RX_TIMEOUT_UUS);
            dwt_setrxaftertxdelay(RESP_TX_TO_FINAL_RX_DLY_UUS);
            state = STATE_wait_FINAL; // state mutate
          }else{
            MSG_TYPE(msg_resp) = MSG_RESP;
            dwt_setrxtimeout(0);
            dwt_setrxaftertxdelay(0);
          }
          int err = sendtx(msg_resp, MSG_RESP_len, DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED, RANGING_ON);
          if (err){
            dwt_setrxtimeout(0);
            dwt_rxenable(DWT_START_RX_IMMEDIATE);
            state = STATE_Receive; // state mutate
            return;
          }
          TRACE_MSG(rx_buffer);
          TRACE_MSG(msg_resp);
          return;

        default: // in STATE_Receive
          trace_msg(rx_buffer);
          dwt_setrxtimeout(0);
          dwt_rxenable(DWT_START_RX_IMMEDIATE);
          return;
      }
      break;

    case STATE_wait_RESP:
      if (event == EVENT_msg_RESP){
        pull_tx_ts = get_tx_ts();
        resp_rx_ts = get_rx_ts();
        MSG_RESP_resp_tx_ts_get(rx_buffer, &resp_tx_ts);
        MSG_RESP_pull_rx_ts_get(rx_buffer, &pull_rx_ts);
  
        float tof = ((resp_rx_ts - pull_tx_ts) - (resp_tx_ts - pull_rx_ts)) / 2;
        float dist = uwb2meters(tof);
        TRACE_MSG(rx_buffer);
        
        DEBUG_transmit_fmt("dist: %f m", dist);
  
        state = STATE_Receive; // state mutate
        dwt_setrxtimeout(0);
        dwt_rxenable(DWT_START_RX_IMMEDIATE);
        return;
      }
      break;

    case STATE_wait_RESP_3: 
      if (event == EVENT_msg_RESP_3){ // тэг получил RESPONCE для 3-смс
          toIdle();

          pull_tx_ts = get_tx_ts();
          resp_rx_ts = get_rx_ts();
          MSG_RESP_pull_rx_ts_get(rx_buffer, &pull_rx_ts);
          MSG_RESP_resp_tx_ts_get(rx_buffer, &resp_tx_ts);
          src_addres = MSG_SRC_ID(rx_buffer);

          final_tx_ts = (resp_rx_ts + (RESP_RX_TO_FINAL_TX_DLY_UUS * UUS_TO_DWT_TIME));
          dwt_setdelayedtrxtime((uint32) (final_tx_ts >> 8) );

          // только там где мы сами вычисляем TX_TS надо + TX_ANT_DELAY
          final_tx_ts = (((int64_t)(final_tx_ts & 0xFFFFFFFE00))) + TX_ANT_DLY;
          MSG_SEQNUM(msg_final)  = frame_seq_nb;
          MSG_PAN_ID(msg_final)  = MY_PAN_ID;
          MSG_DEST_ID(msg_final) = src_addres;
          MSG_SRC_ID(msg_final)  = my_addr;
          MSG_TYPE(msg_final)    = MSG_FINAL;
          MSG_FINAL_resp_rx_ts_set(msg_final, &resp_rx_ts);
          MSG_FINAL_final_tx_ts_set(msg_final, &final_tx_ts);
          MSG_FINAL_pull_tx_ts_set(msg_final, &pull_tx_ts);

          dwt_setrxtimeout(DISTANCE_RX_TIMEOUT_UUS);
          dwt_setrxaftertxdelay(FINAL_TX_TO_DISTANCE_RX_DLY_UUS);
          int err = sendtx(msg_final, MSG_FINAL_len, DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED, RANGING_ON);
          if (err){
            dwt_setrxtimeout(0);
            dwt_rxenable(DWT_START_RX_IMMEDIATE);
            state = STATE_Receive; // state mutate
            return;
          }
          state = STATE_wait_DIST; // state mutate
          return;
      }
      break;

    case STATE_wait_DIST:
      if (event == EVENT_msg_DIST && src_addres == MSG_SRC_ID(rx_buffer)){ // тэг получил DISTANCE от того
        MSG_DIST_dist_get(rx_buffer, &dist);
        // float dist = uwb2meters(tof);
        TRACE_MSG(rx_buffer);
        DEBUG_transmit_fmt("dist: %f m", dist);

        state = STATE_Receive; // state mutate
        dwt_setrxtimeout(0);
        dwt_rxenable(DWT_START_RX_IMMEDIATE);
        return;
      }
      break;

    case STATE_wait_FINAL:
      if (event == EVENT_msg_FINAL && src_addres == MSG_SRC_ID(rx_buffer)){ // якорь получил FINAL от того
        toIdle();

        final_rx_ts = get_rx_ts();
        MSG_FINAL_resp_rx_ts_get(rx_buffer,  &resp_rx_ts);
        MSG_FINAL_final_tx_ts_get(rx_buffer, &final_tx_ts);
        MSG_FINAL_pull_tx_ts_get(rx_buffer, &pull_tx_ts);

        int64_t delay = (final_rx_ts + (FINAL_RX_TO_DISTANCE_TX_DLY_UUS * UUS_TO_DWT_TIME));
        dwt_setdelayedtrxtime((uint32) (delay >> 8) );

        uint64_t Tround1 = resp_rx_ts - pull_tx_ts;
        uint64_t Treply1 = resp_tx_ts - pull_rx_ts;
        uint64_t Treply2 = final_tx_ts - resp_rx_ts;
        uint64_t Tround2 = final_rx_ts - resp_tx_ts;

        double tof = (Tround1*Tround2 - Treply1*Treply2)
            / (double)(final_tx_ts + final_rx_ts - pull_tx_ts - pull_rx_ts); //TODO: исп fixed-point
        float dist = uwb2meters(tof);

        MSG_SEQNUM(msg_dist)  = MSG_SEQNUM(rx_buffer);
        MSG_PAN_ID(msg_dist)  = MY_PAN_ID;
        MSG_DEST_ID(msg_dist) = src_addres;
        MSG_SRC_ID(msg_dist)  = my_addr;
        MSG_DIST_dist_set(msg_dist, &dist);

        int err = sendtx(msg_dist, MSG_RESP_len, DWT_START_RX_DELAYED, RANGING_OFF);
        if (err){
          dwt_setrxtimeout(0);
          dwt_rxenable(DWT_START_RX_IMMEDIATE);
          return;
        }
        state = STATE_Receive; // state mutate
        TRACE_MSG(msg_dist);
        dwt_setrxtimeout(0);
        dwt_rxenable(DWT_START_RX_IMMEDIATE);
        return;
      }
      break;

    case STATE_Sniffer:
      if (EVENT_is(event, EVENTs_msg)){
        // печатает всё что приходит и перед выводит rx_ts
        // магия с перекрывающейся памятью
        //                         5 байт RX_TS + смс
        HAL_StatusTypeDef err = HAL_UART_Transmit(&huart1, rx_buffer_raw, rx_len + 5, 10);
        if (err){
          return;
        }
      }else{
        toIdleFromErr();

        state = STATE_Receive; // state mutate
        dwt_setsniffmode(0, 0, 0); // выключение сниффера
        dwt_enableframefilter(frame_filter); // врубаем фильтрацию обратно
        dwt_setrxtimeout(0);
        dwt_setdblrxbuffmode(0);
        dwt_rxenable(DWT_START_RX_IMMEDIATE);
      }
      return;

    case STATE_none:
      break;
  }
  DEBUG_transmit_fmt("%s/%s", showState(state), showEvent(event));
  src_addres = 0xFFFF;
  state = STATE_Receive;
  dwt_setrxtimeout(0);
  dwt_rxenable(DWT_START_RX_IMMEDIATE);
  return;
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
  // DEBUG_transmit_str("INITED");


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
  // DEBUG_transmit_fmt("I'm % 6s", whoami);

  // Настройка PAN_ID и SHORT_ADDR
  // нужна для работы фильтра
  dwt_setpanid(MY_PAN_ID);
  dwt_setaddress16(my_addr);

  // Confifure filtering
  dwt_enableframefilter(frame_filter); //! пока что что-то идет не так
  
  dwt_configeventcounters(1); // enalbe counters for diagnostics

  // enablt IRQ 
  //                TX OK          RX OK         RX timeout          RX ERR
  dwt_setcallbacks(NULL         , &handler_rxok, &handler_rxtimeout, &handler_rxfailed);
  dwt_setinterrupt(DWT_INT_TFRS | DWT_INT_RFCG | DWT_IRQ_RXTIMEOUT | DWT_IRQ_RXFAILED, 1);

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
    #ifdef SNIFFER_FOR_DEBUG
      static MyEvents saved_event = EVENT_initiate_sniffer;
    #else
      static MyEvents saved_event = EVENT_none;
    #endif

    if (saved_event == EVENT_none && state <= STATE_Receive){
        uint32 cur_tick = HAL_GetTick();

      #ifdef TAG
        static uint32 timer_pull_one = 0; 
        if (cur_tick - timer_pull_one > INITIATE_PULL_ONE_PERIOD_MS){
          timer_pull_one = cur_tick;
          event = EVENT_initiate_pull_3;
          target_indx = 0;
        } else {
          if (target_indx + 1 < targets_len){
            target_indx++;
            if (targets[target_indx] == 0xFFFF)
              continue; // место для брейкпоинта, чтобы считать время всех измерений
            event = EVENT_initiate_pull_3;
          }
        }
      #endif

      #if defined DEBUG_DWT_DIAG && !defined SNIFFER_FOR_DEBUG
        static uint32 timer_diag = 0;
        if (cur_tick - timer_diag > DEBUG_DWT_DIAG_TIMEOUT){
          timer_diag = cur_tick;
          dwt_showDiag(uart_buf);
          Transmit(uart_buf);
          Transmit("\n");
        }
      #endif
    }
    
    // flag handlers and saved event
    if(flag_rxok){
      flag_rxok = 0;
      if (cb_data_p->datalength <= MSG_MAX_LEN){
        rx_len = cb_data_p->datalength;
        *rx_ts_sniffer = get_rx_ts(); // надо rx_ts раньше писать, т.к. uint64_t перекрывает начало пакета
        dwt_readrxdata(rx_buffer, rx_len, 0);
      
      #ifdef ANCHOR
        led_signal((MSG_TYPE(rx_buffer)) & 7);
      #endif

        saved_event = event;
        event = MSG_TYPE_2_EVENT(MSG_TYPE(rx_buffer));

        if (state == STATE_Sniffer){
          // некрасиво конечно, тут проверять это, а не step
          // для двойного буфера
          dwt_rxenable(DWT_START_RX_IMMEDIATE | DWT_NO_SYNC_PTRS);

          uint8_t rxovrr = cb_data_p->status & SYS_STATUS_RXOVRR;
          if (rxovrr){
            Error_Handler();
          }
        }
      }
      //TODO: при ошибке, state остается неизменным и он беск ждёт
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

      if (event == EVENT_rxfail){
        state = STATE_Receive; //? всегда ли
      }
    }

    { // Aka client event

    }

    // All for msg protocols
    if (EVENT_is(event, EVENTs_custom) || EVENT_is(event, EVENTs_msg) || (event == EVENT_rxtimeout)){
      step(event);
    }
    __NOP();
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

#ifdef DEBUG
#define __assert_failed_buf DEBUG_uart_buf
#else
static char __assert_failed_buf[50+1+10+1+15+1];  ///< 50 букв + : + 2^32=10 цифр + : + строка + \0
#endif

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
  snprintf(__assert_failed_buf, sizeof __assert_failed_buf, "%.50s:%lu: Assert failed\n", file, line);
  Transmit(__assert_failed_buf);

  for(int i = 0; i < 3; i++){
    led_signal(1 << 0); HAL_Delay(300);
    led_signal(1 << 1); HAL_Delay(300);
    led_signal(1 << 2); HAL_Delay(300);
  }
  led_signal(7); HAL_Delay(100);

  Error_Handler();
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

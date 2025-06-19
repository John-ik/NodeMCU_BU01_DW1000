#include "main.h"
#include "gpio.h"

#include "port.h"

/**
  * @brief  Checks whether the specified EXTI line is enabled or not.
  * @param  EXTI_Line: specifies the EXTI line to check.
  *   This parameter can be:
  *     @arg EXTI_Linex: External interrupt line x where x(0..19)
  * @retval The "enable" state of EXTI_Line (SET or RESET).
  */
ITStatus EXTI_GetITEnStatus(uint32_t EXTI_Line)
{
    ITStatus bitstatus = RESET;
    uint32_t enablestatus = 0;
    /* Check the parameters */
    assert_param(IS_GET_EXTI_LINE(EXTI_Line));

    enablestatus =  EXTI->IMR & EXTI_Line;
    if (enablestatus != (uint32_t)RESET)
    {
        bitstatus = SET;
    }
    else
    {
        bitstatus = RESET;
    }
    return bitstatus;
}

void led_signal (uint8_t signal){
    if (signal >= (1 << (NUM_LEDS-1))) return;

    switch (signal){
        case 0:
            HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, 0);
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, 0);
            HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 0);
            break;
        case 1:
            HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, 1);
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, 0);
            HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 0);
            break;
        case 2:
            HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, 0);
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, 1);
            HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 0);
            break;
        case 3:
            HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, 1);
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, 1);
            HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 0);
            break;
        case 4:
            HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, 0);
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, 0);
            HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 1);
            break;
        case 5:
            HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, 1);
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, 0);
            HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 1);
            break;
        case 6:
            HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, 0);
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, 1);
            HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 1);
            break;
        case 7:
            HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, 1);
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, 1);
            HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 1);
            break;
    }
}

void reset_DW1000(void)
{
    // // dw1000 data sheet v2.08 §5.6.1 page 20, the RSTn pin should not be driven high but left floating.
    // pinMode(_rst, OUTPUT);
    // digitalWrite(_rst, LOW);
    // delay(2);  // dw1000 data sheet v2.08 §5.6.1 page 20: nominal 50ns, to be safe take more time
    // pinMode(_rst, INPUT);
    // delay(10); // dwm1000 data sheet v1.2 page 5: nominal 3 ms, to be safe take more time
    // // force into idle mode (although it should be already after reset)
    // idle();

    // __HAL_RCC_GPIOB_CLK_ENABLE();
    LL_GPIO_SetPinSpeed(DW_RST_Port, DW_RST_Pin, LL_GPIO_SPEED_FREQ_HIGH);
    // __HAL_RCC_AFIO_CLK_ENABLE();
    LL_GPIO_SetPinMode(DW_RST_Port, DW_RST_Pin, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(DW_RST_Port, DW_RST_Pin, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(DW_RST_Port, DW_RST_Pin, GPIO_NOPULL);
    
    //drive the RSTn pin low
    LL_GPIO_ResetOutputPin(DW_RST_Port, DW_RST_Pin);

    HAL_Delay(2);

    //put the pin back to tri-state ... as input
    LL_GPIO_SetPinMode(DW_RST_Port, DW_RST_Pin, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinPull(DW_RST_Port, DW_RST_Pin, LL_GPIO_PULL_DOWN);

    HAL_Delay(2);

    // goto IDLE. Set TRXOFF bit in SYS_CTRL
    uint32_t tmp = 1 << 6;
	dwt_write32bitreg(0x0D, tmp);
}
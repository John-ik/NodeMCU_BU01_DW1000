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
    if (signal > 3) return;

    switch (signal){
        case 0:
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, 0);
            HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 0);
            break;
        case 1:
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, 1);
            HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 0);
            break;
        case 2:
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, 0);
            HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 1);
            break;
        case 3:
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, 1);
            HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 1);
            break;
    }
}

void reset_DW1000(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// Enable GPIO used for DW1000 reset
	GPIO_InitStructure.Pin = DW1000_RSTn;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(DW1000_RSTn_GPIO, &GPIO_InitStructure);

	//drive the RSTn pin low
	HAL_GPIO_WritePin(DW1000_RSTn_GPIO, DW1000_RSTn, 0);

	//put the pin back to tri-state ... as input
	GPIO_InitStructure.Pin = DW1000_RSTn;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(DW1000_RSTn_GPIO, &GPIO_InitStructure);

    HAL_Delay(2);
}
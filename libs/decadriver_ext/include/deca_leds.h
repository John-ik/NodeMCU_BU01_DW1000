#pragma once

#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_sleep.h"

#define MODE_RESET  0
#define MODE_OUTPUT 1

#define GPIO_RXOKLED 6
#define GPIO_SFDLED  8
#define GPIO_RXLED   10
#define GPIO_TXLED   12
#define GPIO_EXTPA   14
#define GPIO_EXTTXE  16
#define GPIO_EXTRXE  18
#define GPIO_GPIO7   19
#define GPIO_GPIO8   20

void dwt_on_deboundsclock();

void dwt_gpio_mode(uint32 gpio, uint32 mode);

void dwt_blinking_enable();

void dwt_blink_leds();

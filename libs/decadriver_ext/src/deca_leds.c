#include "deca_leds.h"

void dwt_on_deboundsclock(){
    uint32 buf = dwt_read32bitoffsetreg(PMSC_ID, PMSC_CTRL0_OFFSET);
    //                                                        GPDCE       KHZCLKEN
    dwt_write32bitoffsetreg(PMSC_ID, PMSC_CTRL0_OFFSET, buf | (1 << 18) | (1 << 23));
}

void dwt_gpio_mode(uint32 gpio, uint32 mode){
    uint32 buf = dwt_read32bitoffsetreg(GPIO_CTRL_ID, GPIO_MODE_OFFSET);
    dwt_write32bitoffsetreg(GPIO_CTRL_ID, GPIO_MODE_OFFSET, buf | (mode << gpio));
}

void dwt_blinking_enable(){
    uint32 buf = dwt_read32bitoffsetreg(PMSC_ID, PMSC_LEDC_OFFSET);
    dwt_write32bitoffsetreg(PMSC_ID, PMSC_LEDC_OFFSET, buf | PMSC_LEDC_BLNKEN);
}

void dwt_blink_leds(){
    uint32 buf = dwt_read32bitoffsetreg(PMSC_ID, PMSC_LEDC_OFFSET);
    dwt_write32bitoffsetreg(PMSC_ID, PMSC_LEDC_OFFSET, buf | (0x0F << 16));
}

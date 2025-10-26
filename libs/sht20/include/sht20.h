#pragma once


#include "i2c.h"

// RH - relative humidity

#define SHT_i2c_ADDRESS (0x40 << 1) // of the 7-bit I2C device address ‘1000’000'

#define SHT_i2c_WRITE_BIT 0
#define SHT_i2c_READ_BIT  1

#define SHT_i2c_TIMEOUT 100

// Table 6
#define SHT_CMD_TEMP_HOLD 0xE3 // 1110’0011
#define SHT_CMD_RH_HOLD   0xE5 // 1110’0101
#define SHT_CMD_TEMP      0xF3 // 1111’0011
#define SHT_CMD_RH        0xF5 // 1111’0101
#define SHT_CMD_WRITE_REG 0xE6 // 1110’0110
#define SHT_CMD_READ_REG  0xE7 // 1110’0111
#define SHT_CMD_SOFTRESET 0xFE // 1111’1110

enum sht_resolution {
    SHT_RESOLUTION_RH12_T14 = 0,
    SHT_RESOLUTION_RH8_T12,
    SHT_RESOLUTION_RH10_T13,
    SHT_RESOLUTION_RH11_T11
};

// Table 8
#define SHT_REG_RESOLUTION (1 << 7) | (1) // bit 7, 0
#define SHT_REG_END_of_BATTERY (1 << 6) // bit 6
#define SHT_REG_RESERVED (1 << 5) | (1 << 4) | (1 << 3)
#define SHT_REG_HEATER   (1 << 2) // bit 2
#define SHT_REG_DISABLE_OTP (1 << 1) // bit 1

// data
#define SHT_DATA      (~0x3) // 0xFFFC
#define SHT_DATA_BIT0 0x1
#define SHT_DATA_BIT1 0x2

#define SHT_DATA_IS_RH(data) (data & SHT_DATA_BIT1)


// ======================== API ========================

void sht_init(I2C_HandleTypeDef* i2c);
void sht_softreset();
uint8_t sht_read_reg();
void sht_write_reg(uint8_t reg);
uint16_t sht_get_temp_hold();
uint16_t sht_get_rh_hold();
float sht_convert_humidity(uint16_t humidity);
float sht_convert_temp(uint16_t temp);

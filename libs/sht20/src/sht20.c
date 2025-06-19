#include "sht20.h"

#include "stdint.h"

// ==================== LOCAL ====================

static I2C_HandleTypeDef* _sht_i2c;
static uint8_t _sht_i2c_buf[4];
static uint8_t _sht_measurements_mutex;

// ==================== I2C ====================


int sht_i2c_write(uint8_t* p_data, uint16_t data_size){
    uint8_t addr = SHT_i2c_ADDRESS | SHT_i2c_WRITE_BIT;
    return HAL_I2C_Master_Transmit(_sht_i2c, addr, p_data, data_size, SHT_i2c_TIMEOUT);
}

int sht_i2c_read(uint8_t* p_data, uint16_t data_size){
    uint8_t addr = SHT_i2c_ADDRESS | SHT_i2c_READ_BIT;
    return HAL_I2C_Master_Receive(_sht_i2c, addr, p_data, data_size, SHT_i2c_TIMEOUT);
}

void sht_delay(uint32_t ms){
    HAL_Delay(ms);
}

// ==================== API ====================

void sht_init(I2C_HandleTypeDef* i2c){
    _sht_i2c = i2c;
    sht_delay(15);
    sht_softreset();
}

void sht_softreset(){
    _sht_i2c_buf[0] = SHT_CMD_SOFTRESET;
    sht_i2c_write(_sht_i2c_buf, 1);
    sht_delay(15);
}

uint8_t sht_read_reg(){
    _sht_i2c_buf[0] = SHT_CMD_READ_REG;
    sht_i2c_write(_sht_i2c_buf, 1);
    sht_i2c_read(_sht_i2c_buf, 1);
    return _sht_i2c_buf[0];
}

void sht_write_reg(uint8_t reg){
    reg &= SHT_REG_RESERVED; // reset reserved bits
    reg |= (sht_read_reg() & SHT_REG_RESERVED); // read reserved bits

    _sht_i2c_buf[0] = SHT_CMD_WRITE_REG;
    _sht_i2c_buf[1] = reg;
    sht_i2c_write(_sht_i2c_buf, 2);
}

uint16_t sht_get_temp_hold(){
    _sht_i2c_buf[0] = SHT_CMD_TEMP_HOLD;
    sht_i2c_write(_sht_i2c_buf, 1);

    sht_i2c_read(_sht_i2c_buf, 3);
    // TODO: check data type
    return (_sht_i2c_buf[0] << 8) | (_sht_i2c_buf[1]);
    //TODO: check CRC buf[2]
}

uint16_t sht_get_rh_hold(){
    _sht_i2c_buf[0] = SHT_CMD_RH_HOLD;
    sht_i2c_write(_sht_i2c_buf, 1);

    sht_i2c_read(_sht_i2c_buf, 3);
    // TODO: check data type
    return (_sht_i2c_buf[0] << 8) | (_sht_i2c_buf[1]);
    //TODO: check CRC buf[2]
}

float sht_convert_humidity(uint16_t humidity){
    return -6 + 125 * (float)humidity / (1<<16);
}

float sht_convert_temp(uint16_t temp){
    return -46.85 + 175.72 * (float)temp / (1<<16);
}
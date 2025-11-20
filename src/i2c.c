#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include <math.h>
#include <string.h>
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "i2c.h"

void init_i2c() {
    // EEPROM i2c initialization
    i2c_init(EEPROM_i2c, 400000); // fast-mode
    gpio_set_function(EEPROM_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(EEPROM_SCL_PIN, GPIO_FUNC_I2C);
    gpio_set_input_hysteresis_enabled(EEPROM_SDA_PIN, true);
    gpio_set_input_hysteresis_enabled(EEPROM_SCL_PIN, true);
    gpio_set_slew_rate(EEPROM_SDA_PIN, GPIO_SLEW_RATE_SLOW);
    gpio_set_slew_rate(EEPROM_SCL_PIN, GPIO_SLEW_RATE_SLOW);
    gpio_pull_up(EEPROM_SDA_PIN);
    gpio_pull_up(EEPROM_SCL_PIN);

    // DAC i2c initialization
    i2c_init(DAC_i2c, 400000); // fast-mode
    gpio_set_function(DAC_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(DAC_SCL_PIN, GPIO_FUNC_I2C);
    gpio_set_input_hysteresis_enabled(DAC_SDA_PIN, true);
    gpio_set_input_hysteresis_enabled(DAC_SCL_PIN, true);
    gpio_set_slew_rate(DAC_SDA_PIN, GPIO_SLEW_RATE_SLOW);
    gpio_set_slew_rate(DAC_SCL_PIN, GPIO_SLEW_RATE_SLOW);
    gpio_pull_up(DAC_SDA_PIN);
    gpio_pull_up(DAC_SCL_PIN);
}

// Can read/write 4 bytes at a time
void eeprom_write(uint16_t loc, const char* data, uint8_t len){
    if (len > 32) return;

    uint8_t buf[34];
    buf[0] = (uint8_t)(loc >> 8);
    buf[1] = (uint8_t)(loc & 0xFF);
    for (uint8_t i = 0; i < len; i++) buf[2 + i] = (uint8_t)data[i];

    int ret = i2c_write_blocking(EEPROM_i2c, EEPROM_ADDR, buf, len + 2, false);
    if (ret != (int)(len + 2)) return;

    sleep_ms(5);
    return;
}

void eeprom_read(uint16_t loc, char data[], uint8_t len){
    if (len > 32) return;

    uint8_t addr_bytes[2];
    addr_bytes[0] = (uint8_t)(loc >> 8);
    addr_bytes[1] = (uint8_t)(loc & 0xFF);

    int ret = i2c_write_blocking(EEPROM_i2c, EEPROM_ADDR, addr_bytes, 2, true);
    if (ret != 2) return;

    ret = i2c_read_blocking(EEPROM_i2c, EEPROM_ADDR, (uint8_t*)data, len, false);
    if (ret != len) return;

    return;
}

// Can read/write 1 byte at a time
void dac_write(uint8_t reg, uint8_t data){
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = data;

    int ret = i2c_write_blocking(DAC_i2c, DAC_ADDR, buf, 2, false);
    if (ret != 2) return;

    sleep_ms(5);
    return;
}

uint8_t dac_read(uint8_t reg){
    uint8_t value;
    int ret = i2c_write_blocking(i2c1, DAC_ADDR, &reg, 1, true);
    if (ret != 1) return 0;

    ret = i2c_read_blocking(DAC_i2c, DAC_ADDR, &value, 1, false);
    if (ret != 1) return;

    return;
}
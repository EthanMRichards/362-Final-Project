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

// TODO LIST
// Turn off Digital Filter (Register 20)
// EEPROM SETUP (I2C LAB)

void eeprom_write(uint16_t loc, const char* data, uint8_t len){
    if (len > 32) return;

    uint8_t buf[2 + 32];
    buf[0] = (uint8_t)(loc >> 8);
    buf[1] = (uint8_t)(loc & 0xFF);
    for (uint8_t i = 0; i < len; i++) {
        buf[2 + i] = (uint8_t)data[i];
    }

    int ret = i2c_write_blocking(EEPROM_i2c, EEPROM_ADDR, buf, len + 2, false);
    if (ret != (int)(len + 2)) return;

    // wait for write cycle to complete
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

void dac_write(uint16_t loc, const char* data, uint8_t len){
    if (len > 32) return;

    uint8_t buf[2 + 32];
    buf[0] = (uint8_t)(loc >> 8);
    buf[1] = (uint8_t)(loc & 0xFF);
    for (uint8_t i = 0; i < len; i++) {
        buf[2 + i] = (uint8_t)data[i];
    }

    int ret = i2c_write_blocking(DAC_i2c, DAC_WADDR, buf, len + 2, false);
    if (ret != (int)(len + 2)) return;

    // wait for write cycle to complete
    sleep_ms(5);
    return;
}

void dac_read(uint16_t loc, char data[], uint8_t len){
    if (len > 32) return;

    uint8_t addr_bytes[2];
    addr_bytes[0] = (uint8_t)(loc >> 8);
    addr_bytes[1] = (uint8_t)(loc & 0xFF);

    int ret = i2c_write_blocking(DAC_i2c, DAC_RADDR, addr_bytes, 2, true);
    if (ret != 2) return;

    ret = i2c_read_blocking(DAC_i2c, DAC_RADDR, (uint8_t*)data, len, false);
    if (ret != len) return;

    return;
}
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include <math.h>
#include <string.h>
#include "hardware/gpio.h"
#include "hardware/i2c.h"

#define EEPROM_i2c (&i2c0_inst) ///< Identifier for I2C HW Block 0
#define DAC_i2c (&i2c1_inst) ///< Identifier for I2C HW Block 1

#define EEPROM_SDA_PIN 4
#define EEPROM_SCL_PIN 5
#define DAC_SDA_PIN 6
#define DAC_SCL_PIN 7

void init_i2c() {
    // EEPROM i2c initialization
    i2c_init(EEPROM_i2c, 400000);
    gpio_set_function(EEPROM_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(EEPROM_SCL_PIN, GPIO_FUNC_I2C);
    gpio_set_input_hysteresis_enabled(EEPROM_SDA_PIN, true);
    gpio_set_input_hysteresis_enabled(EEPROM_SCL_PIN, true);
    gpio_set_slew_rate(EEPROM_SDA_PIN, GPIO_SLEW_RATE_SLOW);
    gpio_set_slew_rate(EEPROM_SCL_PIN, GPIO_SLEW_RATE_SLOW);
    gpio_pull_up(EEPROM_SDA_PIN);
    gpio_pull_up(EEPROM_SCL_PIN);

    // DAC i2c initialization
    i2c_init(DAC_i2c, 400000);
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

// EEPROM will save the coefficients utilized for the digital equalizer
void eeprom_write(uint16_t loc, const char* data, uint8_t len) {
    
}

void eeprom_read(uint16_t loc, char data[], uint8_t len) {
    
}
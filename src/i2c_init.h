#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include <math.h>
#include <string.h>
#include "hardware/gpio.h"
#include "hardware/i2c.h"

#define EEPROM_i2c (&i2c0_inst) // i2c0 address
#define DAC_i2c (&i2c1_inst)    // i2c1 address

#define EEPROM_SDA_PIN 4
#define EEPROM_SCL_PIN 5
#define DAC_SDA_PIN 6
#define DAC_SCL_PIN 7

void init_i2c(void);

void eeprom_write(uint16_t loc, const char* data, uint8_t len);

void eeprom_read(uint16_t loc, char data[], uint8_t len);
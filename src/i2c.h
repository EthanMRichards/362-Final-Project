#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include <math.h>
#include <string.h>
#include "hardware/gpio.h"

void init_i2c(void);

void eeprom_write(uint16_t loc, const char* data, uint8_t len);

void eeprom_read(uint16_t loc, char data[], uint8_t len);
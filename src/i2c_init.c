#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include <math.h>
#include <string.h>
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "i2c_init.h"

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

void eeprom_write(uint16_t loc, const char* data, uint8_t len) {
    
}

void eeprom_read(uint16_t loc, char data[], uint8_t len) {
    
}

// Wrapper read/write functions for the read/write of the EEPROM and DAC i2c interface
void write(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: write <addr> <data>\n");
        printf("Ensure the address is a hexadecimal number.  No need to include 0x.\n");
        return;
    }
    uint32_t addr = strtol(argv[1], NULL, 16); 
    // concatenate all args from argv[2], until empty string is found, to a string
    char data[32] = "";
    int i = 0;
    int j = 2;
    // Concatenate all args from argv[2] onwards into data, separated by spaces
    for (j = 2; j < argc && i < 32; j++) {
        int arg_len = strlen(argv[j]);
        int copy_len = (i + arg_len < 32) ? arg_len : (32 - i);
        strncpy(&data[i], argv[j], copy_len);
        i += copy_len;
        if (j < argc - 1 && i < 32) {
            data[i++] = ' ';
        }
    }
    data[i] = '\0';
    // ensure addr is a multiple of 32
    if ((addr % 32) != 0) {
        printf("Address 0x%lx is not evenly divisible by 32 (0x20).  Your address must be a hexadecimal value.\n", addr);
        return;
    }
    int msglen = strlen(data);
    if (msglen > 32) {
        printf("Data is too long. Max length is 32.\n");
        return;
    }
    else {
        // pad with spaces so it overwrites the entire 32 bytes
        for (int k = msglen; k < 31; k++) {
            data[k] = ' ';
        }
        data[31] = '\0';
    }
    printf("Writing to address 0x%lx: %s\n", addr, data);
    eeprom_write(addr, data, 32);
}

void read(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: read <addr>\n");
        printf("Ensure the address is a hexadecimal number.  No need to include 0x.\n");
        return;
    }
    uint32_t addr = strtol(argv[1], NULL, 16); 
    char data[32];
    // ensure addr is a multiple of 32
    if ((addr > 15) && (addr < )) {
        printf("Address 0x%lx is not evenly divisible by 32 (0x20).  Your address must be a hexadecimal value.\n", addr);
        return;
    }
    eeprom_read(addr, data, 32);
    printf("String at address 0x%lx: %s\r\n", addr, data);
}
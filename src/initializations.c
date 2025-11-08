#include "initializations.h"
#include "audio.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/pwm.h"
#include "hardware/spi.h"

//////////////////////////////////////////////////////////////////////////////

// initialization functions

// timer interrupts for outputs
void timer_isr(){

}

void timer_init(){

}

// qspi for additional ram and flash (questionably necessary, look at datasheet closer)

// spi for dac chip (spi 0)
void spi_dac_init(){
    gpio_set_function(DAC_SCK, GPIO_FUNC_SPI);
    gpio_set_function(DAC_TX, GPIO_FUNC_SPI);
    spi_init(spi0, 3125000);  // just above the required baudrate (will be accounted for by the isr)
    spi_set_format(spi0, 16, 0, 0, SPI_MSB_FIRST);
    
}

// pwm for dac chip (to "turn" spi into i2s)
void pwm_dac_isr(){
    pwm_clear_irq(pwm_gpio_to_slice_num(DAC_LR));
    // fill spi0 fifo with next four points
    spi0_hw->dr = current.left >> 16; // left high
    spi0_hw->dr = current.left & 0xFF; // left low
    spi0_hw->dr = current.right >> 16; // right high
    spi0_hw->dr = current.right & 0xFF; // right low
    audio_data temp = current;
    current = *current.next; 
    free(&temp);
}

void pwm_dac_init(){
    gpio_set_function(DAC_LR, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(DAC_LR);
    pwm_set_wrap(slice_num, 3125);
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 1563); //figure out later
    pwm_set_enabled(slice_num, true);
    pwm_set_irq_enabled(slice_num, true);
    irq_set_exclusive_handler(PWM_DEFAULT_IRQ_NUM(), pwm_dac_isr);
    irq_set_enabled(PWM_DEFAULT_IRQ_NUM(), true);
}

// i2c for dac control
void i2c_dac_init(){
    gpio_set_function(DAC_SCL, GPIO_FUNC_I2C);
    gpio_set_function(DAC_SDA, GPIO_FUNC_I2C);
    // come back to
}

// spi for sd card and tft (use spi 1)
void spi_inter_init(){
    gpio_set_function(SD_SCK, GPIO_FUNC_SPI); // sck for sd
    gpio_set_function(SD_RX, GPIO_FUNC_SPI); //
    gpio_set_function(SD_TX, GPIO_FUNC_SPI);
    gpio_set_function(DISPLAY_SCK, GPIO_FUNC_SPI);
    gpio_set_function(DISPLAY_TX, GPIO_FUNC_SPI);
    gpio_init(SD_CSn);
    gpio_set_dir(SD_CSn, true);
    gpio_put(SD_CSn, 1);
    gpio_init(DISPLAY_CSn);
    gpio_set_dir(DISPLAY_CSn, true);
    gpio_put(DISPLAY_CSn, 1);
}


// gpio for rotary encoder and other push buttons
void gpio_irq() {

}

void gpio_pins_init(){
    // buttons
    gpio_init(SELECT_PIN);
    gpio_init(BACK_PIN);
    
    // rotary encoder
    gpio_init();
    gpio_init();
}

// dma to take data from the sd card and put it into ram to be accessed efficiently
// another to efficiently take from ram and put it into cache (potentially) to save processing time (come back to this)
// void dma_init(){

// }
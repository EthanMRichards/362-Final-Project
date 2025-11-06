#include "headers.h"

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

// spi for dac chip
void spi_dac_init(){
    gpio_set_function(2, GPIO_FUNC_SPI);
    gpio_set_function(3, GPIO_FUNC_SPI);
    spi_init(spi0, 3125000);  // just above the required baudrate (will be accounted for by the isr)
    spi_set_format(spi0, 16, 0, 0, SPI_MSB_FIRST);
    
}

// pwm for dac chip (to "turn" spi into i2s)
void pwm_dac_isr(){
    if (gpio_get_irq_event_mask(1)){
        gpio_acknowledge_irq(1, GPIO_IRQ_EDGE_RISE);
        // fill spi0 fifo with next four points
        spi0_hw->dr = ; // left high
        spi0_hw->dr = ; // left low
        spi0_hw->dr = ; // right high
        spi0_hw->dr = ; // right low
    }
}

void pwm_dac_init(){
    gpio_set_function(1, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(1);
    pwm_set_wrap(slice_num, 3125);
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 1563); //figure out later
    pwm_set_enabled(slice_num, true);
    pwm_set_irq_enabled(slice_num, true);
    irq_set_exclusive_handler(PWM_DEFAULT_IRQ_NUM(), pwm_dac_isr);
    irq_set_enabled(PWM_DEFAULT_IRQ_NUM(), true);
}

// i2c for dac control
void i2c_dac_init(){
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    // come back to
}

// spi for sd card and tft
void spi_inter_init(){
    gpio_set_function(10, GPIO_FUNC_SPI);
    gpio_set_function(11, GPIO_FUNC_SPI);
    gpio_set_function(12, GPIO_FUNC_SPI);
}

// gpio for rotary encoder and other push buttons
void gpio_pins_init(){

}

// dma to take data from the sd card and put it into ram to be accessed efficiently
// another to efficiently take from ram and put it into cache (potentially) to save processing time (come back to this)
// void dma_init(){

// }
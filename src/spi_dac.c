#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "spi_dac.h"
#include "audio.h"
#include "initializations.h"    

void pwm_dac_isr();

void init_dac(){
    // SPI INIT SDATA
    gpio_set_function(DAC_TX, GPIO_FUNC_SPI);
    gpio_set_function(DAC_SCK, GPIO_FUNC_SPI);
    spi_init(spi0, 3125000); 
    spi_set_format(spi0, 12, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    // PWM INIT for LRCK
    gpio_set_function(DAC_LR, GPIO_FUNC_PWM); // LRCK
    uint slice_num = pwm_gpio_to_slice_num(DAC_LR);
    pwm_set_wrap(slice_num, 3125); // Figure out timings later
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 50 * (1 + (pwm_hw->slice[pwm_gpio_to_slice_num(slice_num[i])].top)) / 100); //duty_cycle * (1 + (pwm_hw->slice[pwm_gpio_to_slice_num(slice_num[i])].top)) / 100
    pwm_set_enabled(slice_num, true);

    // PWM IRQ INIT
    irq_set_exclusive_handler(PWM_DEFAULT_IRQ_NUM(), pwm_dac_isr);
    irq_set_enabled(PWM_DEFAULT_IRQ_NUM(), true);
    pwm_set_irq_enabled(slice_num, true);
}

void pwm_dac_isr(){
    pwm_clear_irq(pwm_gpio_to_slice_num(DAC_LR));
    write_dac_data();
}

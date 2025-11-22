#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "spi_dac.h"
#include "audio.h"

void pwm_dac_isr();

void init_dac(){
    // SPI INIT
    // BCK + SDATA
    gpio_set_function(SPI_DAC_TX, GPIO_FUNC_SPI);
    gpio_set_function(SPI_DAC_SCK, GPIO_FUNC_SPI);
    spi_init(spidac, 3125000); 
    spi_set_format(spi0, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    // PWM INIT 
    // MCLK + LRCK
    gpio_set_function(DAC_LRCK_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(DAC_LRCK_PIN);
    pwm_set_wrap(slice_num, 3125);
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 1563); //figure out later
    pwm_set_enabled(slice_num, true);
    pwm_set_irq_enabled(slice_num, true);
    irq_set_exclusive_handler(PWM_DEFAULT_IRQ_NUM(), pwm_dac_isr);
    irq_set_enabled(PWM_DEFAULT_IRQ_NUM(), true);

    // DMA INIT
    
}

void pwm_dac_isr(){
    pwm_clear_irq(pwm_gpio_to_slice_num(DAC_LRCK_PIN));
    // fill spi0 fifo with next four points
    spi0_hw->dr = current.left >> 16; // left high
    spi0_hw->dr = current.left & 0xFF; // left low
    spi0_hw->dr = current.right >> 16; // right high
    spi0_hw->dr = current.right & 0xFF; // right low
    audio_data temp = current;
    current = *current.next; 
    free(&temp);
}

void send_packet(uint32_t value){

}
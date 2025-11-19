#include "headers.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "biquad.h" 
#include "initializations.h"
#include "audio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "queue.h"
#include "support.h"

void init_wavetable(void);
void set_freq(int chan, float f);
static int duty_cycle = 0;
static int dir = 0;
static int color = 0;
volatile uint16_t volume = 0xFFFF;

int step0 = 0;
int offset0 = 0;
int step1 = 0;
int offset1 = 0;
int volume = 2400;

void init_wavetable(void) {
    for(int i=0; i < N; i++)
        wavetable[i] = (16383 * sin(2 * 3.14 * i / N)) + 16384;
}

void set_freq(int chan, float f) {
    if (chan == 0) {
        if (f == 0.0) {
            step0 = 0;
            offset0 = 0;
        } else
            step0 = (f * N / RATE) * (1<<16);
    }
    if (chan == 1) {
        if (f == 0.0) {
            step1 = 0;
            offset1 = 0;
        } else
            step1 = (f * N / RATE) * (1<<16);
    }
}
void init_volume_pot(void) {
    adc_init();
    adc_gpio_init(40); 
    adc_select_input(0);
}

void init_pwm_audio() {
    duty_cycle = 0;
    gpio_set_function(36,GPIO_FUNC_PWM);
    pwm_set_clkdiv(pwm_gpio_to_slice_num(36),150);
    pwm_set_wrap(pwm_gpio_to_slice_num(36),((1000000 / RATE)-1));
    pwm_set_chan_level(pwm_gpio_to_slice_num(36), PWM_CHAN_A,0 ); 
    init_wavetable();
    pwm_clear_irq(pwm_gpio_to_slice_num(36));
    pwm_set_irq_enabled(pwm_gpio_to_slice_num(36), true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_audio_handler);
    irq_set_enabled(PWM_IRQ_WRAP, true);
    pwm_set_enabled(pwm_gpio_to_slice_num(36), true);

    init_volume_pot();
}
void update_volume_from_pot(void) {
    // 12-bit ADC value: 0..4095
    uint16_t raw = adc_read();

    // Scale 12-bit value up to 16-bit (0..65535) for better resolution
    volume = (uint16_t)(raw << 4);
}
void pwm_audio_handler() {
    uint slice = pwm_gpio_to_slice_num(36);
    pwm_clear_irq(slice);

    offset0 += step0;
    offset1 += step1;

    if (offset0 >= (N << 16)) {
        offset0 -= (N << 16);
    }
    if (offset1 >= (N << 16)) {
        offset1 -= (N << 16);
    }

    // Mix the two wavetable samples
    uint32_t samp = wavetable[offset0 >> 16] + wavetable[offset1 >> 16];
    samp >>= 1;  // divide by 2

    // Apply volume (0..0xFFFF)
    samp = (samp * volume) >> 16;

    // Scale to PWM range
    uint32_t top = pwm_hw->slice[10].top;   // or pwm_hw->slice[slice].top;
    samp = (samp * top) >> 16;

    pwm_set_chan_level(slice, PWM_CHAN_A, (uint16_t)samp);
}
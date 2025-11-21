#include "volume.h"
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


static int duty_cycle = 0;
volatile uint16_t volume = 0xFFFF; 
int step0 = 0;
int offset0 = 0;
int step1 = 0;
int offset1 = 0;

void pwm_audio_handler(void) {
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

    // Mix the two samples
    uint32_t samp = wavetable[offset0 >> 16] + wavetable[offset1 >> 16];
    samp >>= 1;  // divide by 2

    // Take a local copy of volume (in case it changes mid-ISR)
    uint16_t vol = volume;

    // Apply volume: 0..0xFFFF
    samp = (samp * vol) >> 16;

    // Scale to PWM range
    uint32_t top = pwm_hw->slice[slice].top;
    samp = (samp * top) >> 16;

    pwm_set_chan_level(slice, PWM_CHAN_A, (uint16_t)samp);
}

void init_wavetable(void) {
    for (int i = 0; i < N; i++) {
        wavetable[i] = (int16_t)((16383 * sinf(2.0f * 3.14f * i / N)) + 16384);
    }
}

void set_freq(int chan, float f) {
    if (chan == 0) {
        if (f == 0.0f) {
            step0 = 0;
            offset0 = 0;
        } else {
            step0 = (int)((f * N / RATE) * (1 << 16));
        }
    } else if (chan == 1) {
        if (f == 0.0f) {
            step1 = 0;
            offset1 = 0;
        } else {
            step1 = (int)((f * N / RATE) * (1 << 16));
        }
    }
}

void init_volume_pot(void) {
    adc_init();
    adc_gpio_init(45);
    adc_select_input(5);
}

void init_pwm_audio(void) {
    duty_cycle = 0;
    uint slice = pwm_gpio_to_slice_num(36);

    gpio_set_function(36, GPIO_FUNC_PWM);
    pwm_set_clkdiv(slice, 150);
    pwm_set_wrap(slice, ((1000000 / RATE) - 1));
    pwm_set_chan_level(slice, PWM_CHAN_A, 0);

    init_wavetable();

    pwm_clear_irq(slice);
    pwm_set_irq_enabled(slice, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_audio_handler);
    irq_set_enabled(PWM_IRQ_WRAP, true);
    pwm_set_enabled(slice, true);

    init_volume_pot();
}

void update_volume_from_pot(void) {
    adc_select_input(5);
    uint32_t acc = 0;
    const int NSAMPLES = 16;
    for (int i = 0; i < NSAMPLES; i++) {
        acc += adc_read();
    }
    uint16_t raw = acc / NSAMPLES; 
    raw = (raw >> 6) << 6;
    uint16_t new_vol = (uint16_t)(raw << 4);
    const uint16_t THRESH = 0x0100;
    int diff = (int)new_vol - (int)volume;
    if (diff > THRESH || diff < -THRESH) {
        volume = new_vol;
    }
}

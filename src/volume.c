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


eq7_t g_eq;
static int duty_cycle = 0;
volatile uint16_t volume = 0xFFFF; 
int step0 = 0;
int offset0 = 0;
int step1 = 0;
int offset1 = 0;
#define AUDIO_PIN 36    
#define PWM_WRAP 255 
#define TONE_FREQ 1000.0f
void pwm_audio_handler(void) {
    uint slice = pwm_gpio_to_slice_num(AUDIO_PIN);
    pwm_clear_irq(slice);

    offset0 += step0;
    offset1 += step1;

    if (offset0 >= (N << 16)) offset0 -= (N << 16);
    if (offset1 >= (N << 16)) offset1 -= (N << 16);

    uint32_t samp = wavetable[offset0 >> 16] + wavetable[offset1 >> 16];
    samp >>= 1; 
    float x = ((float)samp - 16384.0f) / 16384.0f;
    uint16_t vol = volume;
    float vol_norm = (float)vol / 65535.0f;
    x *= vol_norm;
    float gain_db  = g_eq.gaindB[1];
    float gain_lin = powf(10.0f, gain_db / 20.0f);
    x *= gain_lin;
    float v = x * 0.5f + 0.5f;
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;

    uint32_t top = pwm_hw->slice[slice].top;
    uint16_t level = (uint16_t)(v * top);

    pwm_set_chan_level(slice, PWM_CHAN_A, level);
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
    for (int i = 0; i < 16; i++) {
        acc += adc_read();
    }
    uint16_t raw = acc / 16;
    raw = (raw >> 6) << 6;
    uint16_t new_vol = (uint16_t)(raw << 4);
    int diff = (int)new_vol - (int)volume;
    if (diff > 256 || diff < -256) {
        volume = new_vol;
    }
}


float gain_tester(float prev_gain_db) {
    adc_select_input(5);

    // 1) Oversample
    uint32_t acc = 0;
    const int NSAMPLES = 32;
    for (int i = 0; i < NSAMPLES; i++) {
        acc += adc_read();
    }
    float raw = acc / (float)NSAMPLES;

    // 2) Track min/max we've seen
    static float raw_min = 1e9f;
    static float raw_max = -1e9f;
    if (raw < raw_min) raw_min = raw;
    if (raw > raw_max) raw_max = raw;

    float span = raw_max - raw_min;

    // 3) Map raw -> 0..1 based on observed span
    float t = 0.0f;
    if (span > 1.0f) {  // avoid div-by-zero when we haven't moved the pot yet
        t = (raw - raw_min) / span;   // 0..1 over actual travel
    }

    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    // 4) Map to dB
    float target_gain_db =
        EQ_GAIN_MIN + t * (EQ_GAIN_MAX - EQ_GAIN_MIN);

    // 5) Smoothing
    const float alpha = 0.8f;  // make it a bit more responsive for testing
    float smoothed = alpha * prev_gain_db + (1.0f - alpha) * target_gain_db;

    printf("raw=%.1f  min=%.1f  max=%.1f  t=%.3f  target=%.2f  gain=%.2f\n",
           raw, raw_min, raw_max, t, target_gain_db, smoothed);

    return smoothed;
}
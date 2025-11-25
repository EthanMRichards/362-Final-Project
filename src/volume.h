

#ifndef VOLUME_H
#define VOLUME_H

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include "biquad.h"


extern volatile uint16_t volume;
extern int step0;
extern int offset0;
extern int step1;
extern int offset1;
extern eq7_t g_eq;
void init_pwm_audio(void);
void init_volume_pot(void);
void init_wavetable(void);
void pwm_audio_handler(void);
void set_freq(int chan, float f);
void update_volume_from_pot(void);
float gain_tester(float gain);

#endif
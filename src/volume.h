#ifndef VOLUME_H
#define VOLUME_H

#include <math.h>
#include <stdint.h>
#include <stdio.h>

// If other files need direct access to these, declare them as extern.
// If they should be private, leave them *out* of the header.
extern volatile uint16_t volume;
extern int step0;
extern int offset0;
extern int step1;
extern int offset1;

void init_pwm_audio(void);
void update_volume_from_pot(void);
void init_volume_pot(void);
void init_wavetable(void);
void pwm_audio_handler(void);
void set_freq(int chan, float f);

#endif // VOLUME_H
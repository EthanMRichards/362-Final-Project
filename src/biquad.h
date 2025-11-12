#ifndef BIQUAD.h
#define BIQUAD.h

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"


#define EQ_SAMPLE_RATE 48000.0
static const float EQ_FREQS[7] = {
    50.f, 250.f, 550.f, 1000.f, 2000.f, 4000.f, 10000.f
};
#define EQ_Q_DEFAULT 1.0f
#define EQ_GAIN_MIN (-6.0f)
#define EQ_GAIN_MAX (+6.0f)
typedef struct {
    float a0, a1, a2, b0, b1, b2; // coeffs
} biquadcoeff_t ;

typedef struct {
    float z1, z2; // outputs
} biquadstate_t ;

typedef struct {
    biquadcoeff_t live;     // coefficients used for processing
    biquadcoeff_t target;   // coefficients to glide toward
    biquadstate_t stL;     // left channel state
    biquadstate_t stR;     // right channel state
} band_t;

typedef enum { 
    EQ_PEAK, 
    EQ_LOSHELF, 
    EQ_HISHELF }
 band_type_t;


typedef struct {
    band_t bands[7];
    band_type_t type[7];
    float f0[7]; // center frequencies
    float gaindB[7]; // target gain in dB
    float smooth; // per-sample smoothing
} eq7_t;  

// API CALLS
static float dtft (biquadcoeff_t *c /*coeff structweenar*/, biquadstate_t *s /*current z values (L,R)*/, float x/*X signal of X*/); 
static biquadcoeff_t upper_coeff (float f0, float gaindB); //upper cutoff freq calculation
static biquadcoeff_t lower_coeff (float f0, float gaindB); //lower cutoff freq calc
static biquadcoeff_t peak_coeff (float f0, float gaindB); // center freq
static void eq7_init(eq7_t* e); //init for the struct
static void norm_a0(biquadcoeff_t* c);//a0 normalizer (1)
void eq7_process(eq7_t* e, const float* in, float* out, int nFrames);
void eq7_set_gain(eq7_t* e, int band_idx, float gaindB);
static void updater(band_t* b, band_type_t type, int band_idx, float gaindB);
static inline void lerp_coef(biquadcoeff_t* d, const biquadcoeff_t* t, float k);

#endif
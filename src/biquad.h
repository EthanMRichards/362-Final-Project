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
    float f0[7]; // center freq
    float Q[7]; // q shelf
    float gainDB[7]; // target gain in dB
    float smooth; // per-sample smoothing 
    float sr; // sample rate
} ep7_t;  

// API CALLS
static float dtft (biquadcoeff_t *c /*coeff structweenar*/, biquadstate_t *s /*current z values (L,R)*/, float x/*X signal of X*/); 
static biquadcoeff_t upper_coeff (float sr /*sample rate*/, float f0/*center freq*/, float Q /*q she;f*/, float gainDB /*target gain*/); //upper cutoff freq calculation
static biquadcoeff_t lower_coeff (float sr, float f0, float Q, float gainDB); //lower cutoff freq calc
static biquadcoeff_t peak_coeff (float sr, float f0, float Q, float gainDB); // center freq
static void eq7_init(ep7_t* e, float sr); //init for the struct
static void norm_a0(biquadcoeff_t* c);//a0 normalizer (1)


#endif
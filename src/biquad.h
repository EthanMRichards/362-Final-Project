#ifndef BIQUAD.h
#define BIQUAD.h

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"



typedef struct {
    float a1, a2, b0, b1, b2; // coeffs
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
    float smooth; // 0..1 per-sample smoothing (e.g., 0.995–0.9995)
    float sr; // sample rate
} ep7_t; //eq point  

// API CALLS



#endif
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "biquad.h"


static void norm_a0(biquadcoeff_t* c, float a0) {
    c->b0 /= a0; c->b1 /= a0; c->b2 /= a0;
    c->a1 /= a0; c->a2 /= a0;
}


void eq7_init(ep7_t* e, float sr) {
    memset(e, 0, sizeof(*e));
    e->sr = sr > 0 ? sr : 48000.0;
    e->smooth = 0.9975f; // ~15–20 ms glide @ 48k
    // Suggested centers
    float freqs[7] = {50.f, 250.f, 550.f, 1000.f, 2000.f, 4000.f, 10000.f};
    for (int i=0; i<7; ++i) {
        e->type[i] = (i==0) ? EQ_LOSHELF : (i==6 ? EQ_HISHELF : EQ_PEAK);
        e->f0[i] = freqs[i];
        e->Q[i] = 1.0f;
        e->gainDB[i] = 0.0f;
        update_target(&e->bands[i], e->type[i], e->sr, e->f0[i], e->Q[i], e->gainDB[i]);
        e->bands[i].live = e->bands[i].target; // start with no glide
    }
}

biquadcoeff_t peak_coeff (float sr, float f0, float Q, float gainDB) {
    biquadcoeff_t c;
    float A = powf(10, gainDB/40);
    float w0 = (2*3.14159265358979323846) / (f0/sr); 
    float sn  = sinf(w0); 
    float cs = cosf(w0);
    float alpha = sn / (2.0f * Q);
    c.b0 = 1 + alpha * A;
    c.b1 = -2 * cs;
    c.b2 = 1 - alpha*A;
    c.a1 = -2 * cs;
    c.a2 = 1 - alpha / A;
    return c; 
}

biquadcoeff_t lower_coeff (float sr, float f0, float Q, float gainDB) {
    biquadcoeff_t b;
    float A = powf(10, gainDB/40);
    float w0 = (2*3.14159265358979323846) / (f0/sr); 
    float sn  = sinf(w0); 
    float cs = cosf(w0);
    float alpha = sn / (2.0f * Q);
    b.b0 = A *( (A + 1) - (A - 1) * cs + 2 * sqrtf(A)*alpha); 
    b.b1 = 2*A*( (A - 1) - (A + 1) * cs); 
    b.b2 =  A *( (A + 1) - (A - 1) * cs - 2 * sqrtf(A)*alpha); 
    b.a1 = -2*((A-1) + (A+1) * cs);
    b.a2 = A *( (A + 1) + (A - 1) * cs - 2 * sqrtf(A)*alpha); 
    return b; 
}

biquadcoeff_t upper_coeff (float sr, float f0, float Q, float gainDB) {
    biquadcoeff_t d;
    float A = powf(10, gainDB/40);
    float w0 = (2*3.14159265358979323846) / (f0/sr); 
    float sn  = sinf(w0); 
    float cs = cosf(w0);
    float alpha = sn / (2.0f * Q);
    d.b0 = A *( (A + 1) + (A - 1) * cs + 2 * sqrtf(A)*alpha); 
    d.b1 = -2*A*( (A - 1) + (A + 1) * cs); 
    d.b2 =  A *( (A + 1) + (A - 1) * cs - 2 * sqrtf(A)*alpha); 
    d.a1 = 2*((A-1) - (A+1) * cs);
    d.a2 = A *( (A + 1) - (A - 1) * cs - 2 * sqrtf(A)*alpha); 
    return d; 
}




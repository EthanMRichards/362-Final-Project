#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "biquad.h"

//Biquad filter is recursive formula, taking the form of this in the time domain 
// y[n]=b_{0}x[n]+b_{1}x[n-1]+b_{2}x[n-2]-a_{1}y[n-1]-a_{2}y[n-2]} assuming a0 is normalized
#define EQ_SAMPLE_RATE 48000.0
static const float EQ_FREQS[7] = {
    50.f, 250.f, 550.f, 1000.f, 2000.f, 4000.f, 10000.f
};
#define EQ_Q_DEFAULT 1.0f
#define EQ_GAIN_MIN (-6.0f)
#define EQ_GAIN_MAX (+6.0f)

static void norm_a0(biquadcoeff_t* c) {
    c->b0 /= c->a0; c->b1 /= c->a0; c->b2 /= c->a0;
    c->a1 /= c->a0; c->a2 /= c->a0;
}


static biquadcoeff_t peak_coeff (float sr, float f0, float Q, float gaindB) { //center freq calculation
    biquadcoeff_t c;
    float A = powf(10, gaindB/40); //gain
    float w0 = (2*3.14159265358979323846) / (f0/sr);  //freq
    float sn  = sinf(w0); 
    float cs = cosf(w0);
    float alpha = sn / (2.0f * Q);
    c.b0 = 1 + alpha * A;
    c.b1 = -2 * cs;
    c.b2 = 1 - alpha*A;
    c.a0 = 1 + alpha/A;
    c.a1 = -2 * cs;
    c.a2 = 1 - alpha / A;
    norm_a0(&c);
    return c; 
}

static biquadcoeff_t lower_coeff (float sr, float f0, float Q, float gaindB) { //lower freq calculation
    biquadcoeff_t c;
    float A = powf(10, gaindB/40); 
    float w0 = (2*3.14159265358979323846) / (f0/sr); 
    float sn  = sinf(w0); 
    float cs = cosf(w0);
    float alpha = sn / (2.0f * Q);
    c.b0 = A *( (A + 1) - (A - 1) * cs + 2 * sqrtf(A)*alpha); 
    c.b1 = 2*A*( (A - 1) - (A + 1) * cs); 
    c.b2 =  A *( (A + 1) - (A - 1) * cs - 2 * sqrtf(A)*alpha); 
    c.a0 =  A *( (A + 1) + (A - 1) * cs - 2 * sqrtf(A)*alpha); 
    c.a1 = -2*((A-1) + (A+1) * cs);
    c.a2 = A *( (A + 1) + (A - 1) * cs - 2 * sqrtf(A)*alpha); 
    norm_a0(&c);
    return c; 
}

static biquadcoeff_t upper_coeff (float sr, float f0, float Q, float gaindB) { //upper shelf calculation 
    biquadcoeff_t c;
    float A = powf(10, gaindB/40);
    float w0 = (2*3.14159265358979323846) / (f0/sr); 
    float sn  = sinf(w0); 
    float cs = cosf(w0);
    float alpha = sn / (2.0f * Q);
    c.b0 = A *( (A + 1) + (A - 1) * cs + 2 * sqrtf(A)*alpha); 
    c.b1 = -2*A*( (A - 1) + (A + 1) * cs); 
    c.b2 =  A *( (A + 1) + (A - 1) * cs - 2 * sqrtf(A)*alpha); 
    c.a0 =  A *( (A + 1) - (A - 1) * cs - 2 * sqrtf(A)*alpha); 
    c.a1 = 2*((A-1) - (A+1) * cs);
    c.a2 = A *( (A + 1) - (A - 1) * cs - 2 * sqrtf(A)*alpha); 
    norm_a0(&c);
    return c; 
}

static float dtft (biquadcoeff_t *c, biquadstate_t *s, float x) {
    float y = c->b0 + s->z1;
    s->z1 = c->b1*x - c->a1*y + s->z2;
    s-> z2 = c->b2 * x - c->a2*y;
    return y;
}


static void eq7_init(ep7_t* e, float sr) {
    memset(e, 0, sizeof(*e));
    e->sr = sr > 0 ? sr : 48000.0;
    e->smooth = 0.9975f; // ~15–20 ms glide @ 48k
    //band Centers
    for (int i=0; i<7; ++i) {
        e->type[i] = (i==0) ? EQ_LOSHELF : (i==6 ? EQ_HISHELF : EQ_PEAK);
        e->f0[i] = EQ_FREQS[i];
        e->Q[i] = 1.0f;
        e->gaindB[i] = 0.0f;
        update_target(&e->bands[i], e->type[i], e->sr, e->f0[i], e->Q[i], e->gaindB[i]);
        e->bands[i].live = e->bands[i].target; // start with no glide
    }
}

static void updater(band_t* b, band_type_t type, float sr, int band_idx, float gaindB) {

    if ((unsigned)band_idx >= 7) return;

    // Clamp gain
    if (gaindB > EQ_GAIN_MAX) gaindB = EQ_GAIN_MAX;
    if (gaindB < EQ_GAIN_MIN) gaindB = EQ_GAIN_MIN;

    float f0 = EQ_FREQS[band_idx];
    float Q  = EQ_Q_DEFAULT;

    biquadcoeff_t tmp;
    switch (type) {
        case EQ_PEAK:
            tmp = peak_coeff(EQ_SAMPLE_RATE, f0, Q, gaindB);
            break;
        case EQ_LOSHELF:
            tmp = lower_coeff(EQ_SAMPLE_RATE, f0, 1.0f, gaindB);
            break;
        case EQ_HISHELF:
            tmp = upper_coeff(EQ_SAMPLE_RATE, f0, 1.0f, gaindB);
            break;
    }
    b->target = tmp;
}
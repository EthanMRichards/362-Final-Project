#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "biquad.h"

//Biquad filter is recursive formula, taking the form of this in the time domain 
// y[n]=b_{0}x[n]+b_{1}x[n-1]+b_{2}x[n-2]-a_{1}y[n-1]-a_{2}y[n-2]} assuming a0 is normalized


//helper functions

//   Normalize a biquad so that a0 == 1 by dividing all coefficients
//   (b0,b1,b2,a1,a2) by the current a0. This avoids an extra divide per sample
void norm_a0(biquadcoeff_t* c) {
    c->b0 /= c->a0; c->b1 /= c->a0; c->b2 /= c->a0;
    c->a1 /= c->a0; c->a2 /= c->a0;
}

// Peaking BQ formula
biquadcoeff_t peak_coeff ( float f0, float gaindB) { //center freq calculation
    biquadcoeff_t c;
    int Q = 1;
    float A = powf(10, gaindB/40); //gain
    float w0 = (2*3.14159265358979323846) * (f0/EQ_SAMPLE_RATE);  //freq
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
//  Build a low-shelf biquad at corner f0 (Hz) with gain gaindB (dB).
biquadcoeff_t lower_coeff (float f0, float gaindB) { //lower freq calculation
    biquadcoeff_t c;
    int Q = 1; 
    float A = powf(10, gaindB/40); 
    float w0 = (2*3.14159265358979323846) * (f0/EQ_SAMPLE_RATE); 
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
//  Build a high-shelf biquad at corner f0 (Hz) with gain gaindB (dB).
biquadcoeff_t upper_coeff (float f0, float gaindB) { //upper shelf calculation 
    biquadcoeff_t c;
    int Q = 1;
    float A = powf(10, gaindB/40);
    float w0 = (2*3.14159265358979323846) * (f0/EQ_SAMPLE_RATE); 
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

// Linear interpolation of biquad coefficients toward target, x input, y recursive output, z1 is channel 1, z2 is channel 2, Biquad is 2nd order
float dtft (biquadcoeff_t *c, biquadstate_t *s, float x) {
    float y = c->b0 *x + s->z1;
    s->z1 = c->b1*x - c->a1*y + s->z2;
    s-> z2 = c->b2 * x - c->a2*y;
    return y;
}

// -----------------------------------------


//  Initialize the EQ object:
void eq7_init(eq7_t* e) {
    memset(e, 0, sizeof(*e));
    e->smooth = 0.9975f; // ~15–20 ms glide @ 48k
    for (int i=0; i<7; ++i) {
        e->type[i] = (i==0) ? EQ_LOSHELF : (i==6 ? EQ_HISHELF : EQ_PEAK);
        e->f0[i] = EQ_FREQS[i];
        e->gaindB[i] = 0.0f;
        updater(&e->bands[i], e->type[i], i, e->gaindB[i]);
        e->bands[i].live = e->bands[i].target; // start with no glide
    }
}
//   With k close to 1.0 (e.g., 0.9975), this glides coefficients over a few ms
//   to prevent audible clicks when parameters change.
 void lerp_coef(biquadcoeff_t* d, const biquadcoeff_t* t, float k) {
    float ik = 1.0f - k;
    d->b0 = k*d->b0 + ik*t->b0;
    d->b1 = k*d->b1 + ik*t->b1;
    d->b2 = k*d->b2 + ik*t->b2;
    d->a1 = k*d->a1 + ik*t->a1;
    d->a2 = k*d->a2 + ik*t->a2;
}
//   Rebuild the target coefficients for a single band given:
//     - band type (peak / low shelf / high shelf)
//     - band index (0..6) to choose f0 from EQ_FREQS
//     - desired gain in dB (clamped to EQ_GAIN_MIN..EQ_GAIN_MAX)
//   The ISR will smoothly glide from current 'live' to new 'target'.
void updater(band_t* b, band_type_t type, int band_idx, float gaindB) {

    if ((unsigned)band_idx >= 7) return;
    if (gaindB > EQ_GAIN_MAX) gaindB = EQ_GAIN_MAX;
    if (gaindB < EQ_GAIN_MIN) gaindB = EQ_GAIN_MIN;

    float f0 = EQ_FREQS[band_idx];

    biquadcoeff_t tmp;
    switch (type) {
        case EQ_PEAK:
            tmp = peak_coeff(f0, gaindB);
            break;
        case EQ_LOSHELF:
            tmp = lower_coeff(f0, gaindB);
            break;
        case EQ_HISHELF:
            tmp = upper_coeff(f0, gaindB);
            break;
    }
    b->target = tmp; // one struct write; ISR will lerp live->target each sample
}
void eq7_set_gain(eq7_t* e, int band_idx, float gaindB) {
    if ((unsigned)band_idx >= 7) return;
    // Store clamped gain for UI/state
    e->gaindB[band_idx] = fmaxf(EQ_GAIN_MIN, fminf(EQ_GAIN_MAX, gaindB));
    // Recompute target coefficients for this band
    updater(&e->bands[band_idx], e->type[band_idx], band_idx, e->gaindB[band_idx]);
}

//   Process an interleaved stereo buffer in blocks (L,R)
void eq7_process(eq7_t* e, const float* in, float* out, int nFrames) {
    const float k = e->smooth;
    for (int n=0; n<nFrames; ++n) {
        float L = in[2*n + 0];
        float R = in[2*n + 1];
        // Run through all 7 bands
        for (int b=0; b<7; ++b) {
            // Smoothly morph coefficients (avoids zipper/clicks)
            lerp_coef(&e->bands[b].live, &e->bands[b].target, k);
            // Filter left and right channels (separate states)
            L = dtft(&e->bands[b].live, &e->bands[b].stL, L);
            R = dtft(&e->bands[b].live, &e->bands[b].stR, R);
        }
        out[2*n + 0] = L;
        out[2*n + 1] = R;
    }
}
float eq7_process_mono(eq7_t* e, float x) {
    float y = x;
    const float k = e->smooth;

    for (int b = 0; b < 7; ++b) {
        lerp_coef(&e->bands[b].live, &e->bands[b].target, k);
        y = dtft(&e->bands[b].live, &e->bands[b].stL, y);  // use left state as mono
    }

    return y;
}

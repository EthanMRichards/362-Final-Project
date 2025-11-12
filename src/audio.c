#include "headers.h"
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/spi.h"
#include "hardware/irq.h"
#include "hardware/timer.h"
#include "biquad.h"
#include "initializations.h"

// ---------- Configuration (MUST match codec/DMA configuration) ----------
#define FS_HZ 48000
#define FRAMES_PER_BLOCK 128
#define STEREO 2
#define PROC_RING_BLOCKS 8 

__attribute__((weak)) bool audio_src_init(const char* path, uint32_t target_fs) { (void)path; (void)target_fs; return false; } // MATT PLEASE MAKE THIS MATCH OR MY SHIT DOESNT WORK
__attribute__((weak)) void audio_src_task(void) {}
__attribute__((weak)) bool audio_src_pop_block(int16_t out[FRAMES_PER_BLOCK * STEREO]) { (void)out; return false; }

typedef struct {
    int16_t data[FRAMES_PER_BLOCK * STEREO];
} block_i16_t;


static block_i16_t ring[PROC_RING_BLOCKS];
static _Atomic uint32_t head = 0; // write by Core1 (DSP)
static _Atomic uint32_t tail = 0; // read by Core0 (DAC)
// --------------Produce Consumer Atomics for stack management
static inline bool processed_push(const int32_t *src) {
    uint32_t h = atomic_load_explicit(&head, memory_order_relaxed);
    uint32_t t = atomic_load_explicit(&tail, memory_order_acquire);
    if (h - t >= PROC_RING_BLOCKS) return false;
    memcpy(ring[h % PROC_RING_BLOCKS].data,
           src, sizeof(int32_t) * FRAMES_PER_BLOCK * STEREO);
    atomic_store_explicit(&head, h + 1, memory_order_release);
    return true;
}

static inline bool processed_pop(int32_t *dst) {
    uint32_t t = atomic_load_explicit(&tail, memory_order_relaxed);
    uint32_t h = atomic_load_explicit(&head, memory_order_acquire);
    if (t == h) return false;
    memcpy(dst, ring[t % PROC_RING_BLOCKS].data,
           sizeof(int32_t) * FRAMES_PER_BLOCK * STEREO);
    atomic_store_explicit(&tail, t + 1, memory_order_release);
    return true;
}
static eq7_t g_eq;

// Working float buffers shared between cores (ping/pong)
static float in_f [2][FRAMES_PER_BLOCK * STEREO];
static float out_f[2][FRAMES_PER_BLOCK * STEREO];
static volatile int last_filled = -1;

// Convert float block to int16 interleaved
static void floats_to_i16(const float *in, int16_t *out) {
    for (int n=0;n<FRAMES_PER_BLOCK;++n) {
        float L = in[2*n+0]; if (L>1) L=1; if (L<-1) L=-1;
        float R = in[2*n+1]; if (R>1) R=1; if (R<-1) R=-1;
        out[2*n+0] = (int16_t)lrintf(L * 32767.0f);
        out[2*n+1] = (int16_t)lrintf(R * 32767.0f);
    }
}

// ---------------- Core 1 worker ----------------
static void core1_main(void) {
    while (1) {
        // Wait for buffer index (0/1) handed by Core 0
        uint32_t idx = multicore_fifo_pop_blocking();
        // Run EQ
        eq7_process(&g_eq, in_f[idx], out_f[idx], FRAMES_PER_BLOCK);
        // Convert to int16 and push into processed-output ring
        int16_t tmp[FRAMES_PER_BLOCK * STEREO];
        floats_to_i16(out_f[idx], tmp);
        while (!processed_push(tmp)) {
            tight_loop_contents();
        }
        multicore_fifo_push_blocking(idx);
        last_filled = (int)idx;
    }
}
// ---------------- Core 0 DAC output and feeding Core 1 ----------------
static inline void dac_write_sample(int16_t s) {
    uint16_t word = (uint16_t)s;
    uint8_t bytes[2] = { (uint8_t)(word >> 8), (uint8_t)(word & 0xFF) };
    spi_write_blocking(spi0, bytes, 2);
}

// 48 kHz timer ISR: write one stereo frame each tick
static bool __not_in_flash_func(dac_timer_cb)(repeating_timer_t *t) {
    (void)t;
    static int16_t blk[FRAMES_PER_BLOCK * STEREO];
    static int sample_idx = FRAMES_PER_BLOCK; // force initial pop
    static bool have_block = false;
    if (sample_idx >= FRAMES_PER_BLOCK) {
        // Try to pop a new processed block
        have_block = processed_pop(blk);
        sample_idx = 0;
        if (!have_block) {
            dac_write_sample(0); // L
            dac_write_sample(0); // R
            return true;
        }
    }
    int16_t L = blk[2*sample_idx + 0];
    int16_t R = blk[2*sample_idx + 1];
    sample_idx++;
    dac_write_sample(L);
    dac_write_sample(R);
    return true;
}

static void feed_core1_with_input_blocks(void) {
    static int idx = 0;
    // Pull one block from SD source (or fill zeros)
    int16_t in_i16[FRAMES_PER_BLOCK * STEREO];
    if (!audio_src_pop_block(in_i16)) {
        memset(in_i16, 0, sizeof(in_i16));
    }
    // Convert to float for EQ input buffers shared with Core 1
    for (int n=0; n<FRAMES_PER_BLOCK; ++n) {
        in_f[idx][2*n+0] = in_i16[2*n+0] * (1.0f/32768.0f);
        in_f[idx][2*n+1] = in_i16[2*n+1] * (1.0f/32768.0f);
    }
    // Hand this index to Core 1 (non-blocking preferred)
    if (!multicore_fifo_push_timeout_us((uint32_t)idx, 0)) {
        return;
    }
    while (multicore_fifo_rvalid()) (void)multicore_fifo_pop_blocking();

    idx ^= 1; // toggle 0↔1
}
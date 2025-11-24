#include "initializations.h"
#include "audio.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/pwm.h"
#include "hardware/spi.h"
#include "ff.h"
#include "audio.h"
#include "initializations.h"

// ----------------------------------- SD CARD ----------------------------

#define FS_HZ 48000
#define FRAMES_PER_BLOCK 128
#define STEREO 2
#define PROC_RING_BLOCKS 8 

#define SD_MISO 12
#define SD_CS 13
#define SD_SCK 14
#define SD_MOSI 15

// Redundant, look at display.c for the initialization function
// void init_spi_sdcard() {
//     // fill in.
//     gpio_set_dir(SD_SCK, true);
//     gpio_set_dir(SD_CS, true);
//     gpio_set_dir(SD_MISO, false);
//     gpio_set_dir(SD_MOSI, true);

//     gpio_set_function(SD_SCK, GPIO_FUNC_SPI);
//     gpio_set_function(SD_CS, GPIO_FUNC_SIO);
//     gpio_set_function(SD_MISO, GPIO_FUNC_SPI);
//     gpio_set_function(SD_MOSI, GPIO_FUNC_SPI);

//     gpio_put(SD_CS, true);

//     spi_init(spi1, 400000);
//     spi_set_format(spi1, 8, 0, 0, SPI_MSB_FIRST);
// }

void disable_sdcard() {
    // fill in.
    uint16_t buffer = 0xFF;

    gpio_put(SD_CSn, true);
    spi_write_blocking(spi1, &(buffer), 1);

    gpio_set_function(SD_TX, GPIO_FUNC_SIO);
    gpio_put(SD_TX, true);
}

void enable_sdcard() {
    // fill in.
    gpio_set_function(SD_TX, GPIO_FUNC_SPI);
    gpio_put(SD_CSn, false);
}

void sdcard_io_high_speed() {
    // fill in.
    spi_set_baudrate(spi1, 12000000);
}

void init_sdcard_io() {
    // fill in.
    init_spi_sdcard();
    disable_sdcard();
}

static int16_t pcm_blockA[FRAMES_PER_BLOCK * STEREO];
static int16_t pcm_blockB[FRAMES_PER_BLOCK * STEREO];

static volatile bool blockA_ready = false;
static volatile bool blockB_ready = false;

static FIL audio_file;
static bool use_blockA = true;

FATFS fatfs;

#define PCM_BLOCK_BYTES (FRAMES_PER_BLOCK * STEREO * sizeof(int16_t))

static bool audio_src_init(const char* path, uint32_t target_fs) { 
     FRESULT fr;

    init_sdcard_io();

    fr = f_mount(&fatfs, "", 1);
    if (fr != FR_OK) return false;

    fr = f_open(&audio_file, path, FA_READ);
    if (fr != FR_OK) return false;

    // skip WAV header if needed
    // f_lseek(&audio_file, 44);

    UINT br;
    f_read(&audio_file, pcm_blockA, PCM_BLOCK_BYTES, &br);
    blockA_ready = (br == PCM_BLOCK_BYTES);

    f_read(&audio_file, pcm_blockB, PCM_BLOCK_BYTES, &br);
    blockB_ready = (br == PCM_BLOCK_BYTES);

    return true;
}

void audio_src_task(void) {
    UINT br;

    if (!blockA_ready) {
        f_read(&audio_file, pcm_blockA, PCM_BLOCK_BYTES, &br);
        if (br == 0) {
            memset(pcm_blockA, 0, PCM_BLOCK_BYTES);
        }
        blockA_ready = true;
    }

    if (!blockB_ready) {
        f_read(&audio_file, pcm_blockB, PCM_BLOCK_BYTES, &br);
        if (br == 0) {
            memset(pcm_blockB, 0, PCM_BLOCK_BYTES);
        }
        blockB_ready = true;
    }
}

bool audio_src_pop_block(int16_t out[FRAMES_PER_BLOCK * STEREO]) {

    if (blockA_ready) {
        memcpy(out, pcm_blockA, PCM_BLOCK_BYTES);
        blockA_ready = false;
        return true;
    }

    if (blockB_ready) {
        memcpy(out, pcm_blockB, PCM_BLOCK_BYTES);
        blockB_ready = false;
        return true;
    }

    return false;
}
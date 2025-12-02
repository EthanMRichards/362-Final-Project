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
#include "sdcard.h"
#include "lcd.h"
#include "diskio.h"

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

FATFS fs_storage; // Global file system object

int hovered_item = 0;
int selected_item = -1;

#define MAX_ENTRIES 15
DirEntry dir_list[MAX_ENTRIES];
int dir_count = 0;
DIR dir;
FILINFO fno;

int root_depth = 0;
int background_color = LGRAYBLUE;
int spacing = 40;
int margin = 20;

#define PCM_BLOCK_BYTES (FRAMES_PER_BLOCK * STEREO * sizeof(int16_t))

static bool audio_src_init(const char* path, uint32_t target_fs) { // this function is probably not used, the sd card init/mounting is done at the beginning of main and file opening is done on select
     FRESULT fr;

    init_sdcard_io();

    fr = f_mount(&fs_storage, "", 1);
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

void init_spi_lcd() {
    gpio_set_function(DISPLAY_CSn, GPIO_FUNC_SIO);
    gpio_set_function(DISPLAY_DC, GPIO_FUNC_SIO);
    gpio_set_function(DISPLAY_RST, GPIO_FUNC_SIO);

    gpio_set_dir(DISPLAY_CSn, GPIO_OUT);
    gpio_set_dir(DISPLAY_DC, GPIO_OUT);
    gpio_set_dir(DISPLAY_RST, GPIO_OUT);

    gpio_put(DISPLAY_CSn, 1); // CS high
    gpio_put(DISPLAY_DC, 0); // DC low
    gpio_put(DISPLAY_RST, 1); // nRESET high

    // initialize SPI0 with 48 MHz clock
    gpio_set_function(DISPLAY_SCK, GPIO_FUNC_SPI);
    gpio_set_function(DISPLAY_TX, GPIO_FUNC_SPI);
    spi_init(spi0, 1 * 1000 * 1000);
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST); // REMINDER TO MYSELF THAT I CHANGED IT TO SPI0, ASK ABOUT WHAT PUTTING BOTH ON SPI1 ACTUALLY LOOKS LIKE
}

void mount_sd()
{
    FATFS *fs = &fs_storage;
    int res = f_mount(fs, "", 1);
    if (res != FR_OK){
        printf("Error occurred while mounting");
    } else{
        f_chdir("/");
    }
}

void open_file(){
    FIL fil;        /* File object */
    char line[100]; /* Line buffer */
    FRESULT fr;     /* FatFs return code */
    char filename[30];

    strcpy(filename, dir_list[selected_item].name);

    printf("FILE NAME: \n");
    printf(filename);
    printf("\n");

    // TXT SPECIFIC:
    /* Open a text file */
    fr = f_open(&fil, filename, FA_READ);
    if (fr) {
        print_error(fr, filename);
        return;
    }

    /* Read every line and display it */
    while(f_gets(line, sizeof line, &fil))
        printf(line);

    // END OF TXT SPECIFIC

    // WAV SPECIFIC:
    // fr = f_open(&fil, filename, FA_READ);
    // if (fr) {
    //     print_error(fr, filename);
    //     return;
    // }
    // skip WAV header if needed
    // f_lseek(&audio_file, 44);
    // UINT br;
    // f_read(&audio_file, pcm_blockA, PCM_BLOCK_BYTES, &br);
    // blockA_ready = (br == PCM_BLOCK_BYTES);

    // f_read(&audio_file, pcm_blockB, PCM_BLOCK_BYTES, &br);
    // blockB_ready = (br == PCM_BLOCK_BYTES);
    // audio_src_task();

    /* Close the file */
    printf("\n\n");
    f_close(&fil);
}

void list_directory(void) {
    FRESULT fr;

    dir_count = 0;

    fr = f_opendir(&dir, ".");
    if (fr != FR_OK) {
        printf("Error opening directory: %d\n", fr);
        return;
    }

    while (1) {
        fr = f_readdir(&dir, &fno);
        if (fr != FR_OK || fno.fname[0] == 0)
            break;

        if (strcmp(fno.fname, ".") == 0 || strcmp(fno.fname, "..") == 0)
            continue;

        if (strcmp(fno.fname, "SYSTEM~1") != 0){
            strncpy(dir_list[dir_count].name, fno.fname, sizeof(dir_list[dir_count].name));
            dir_list[dir_count].attr = fno.fattrib;
            dir_count++;
        }

        if (dir_count >= MAX_ENTRIES){
            break;
        }
    }

    f_closedir(&dir);

    // printf("Contents of current directory:\n");
    // for (int i = 0; i < dir_count; i++) {
    //     printf("[%2d] %s%s\n", i,
    //         dir_list[i].name,
    //         (dir_list[i].attr & AM_DIR) ? "/" : "");
    // }
}

bool is_directory(DirEntry *e) {
    return (e->attr & AM_DIR) != 0;
}

// _________________________Move Cursor Functions___________________
void tft_draw_item_list(){
    int i;
    LCD_DrawFillRectangle(35, 15, 150, 250, background_color);
    for (i = 0; i < dir_count; i++){
        LCD_DrawString(45, (i*spacing) + margin, 0x0867, LGRAYBLUE, dir_list[i].name, 16, 1);
    }
}

void tft_draw_pointer(){
    LCD_DrawFillRectangle(160, 15, 225, 250, background_color);
    LCD_DrawFillTriangle(200, margin + hovered_item*spacing, 200, margin + 16 + hovered_item*spacing, 180, margin + 8 + hovered_item*spacing, RED);
}

void tft_move_cursor(bool right){ 
    // Check if curser will scroll past the first/last song if so return and don't update potision
    // Otherwise update position
    if (right){
        if ((hovered_item + 1) < dir_count){
            hovered_item++;
            tft_draw_pointer();
        }
    } else {
        if (hovered_item > 0){
            hovered_item--;
            tft_draw_pointer();       
        }
    }
}

void tft_select_item(){ 
    // Self explainatory, start playing new song if its hovering over it

    selected_item = hovered_item;

    DirEntry *e = &dir_list[selected_item];

    if (is_directory(e)) {
        // Enter the folder
        printf("Entering directory: %s\n", e->name);
        if (f_chdir(e->name) == FR_OK) {
            list_directory();
            tft_draw_item_list();
            selected_item = -1;
            hovered_item = 0;
            root_depth++;
            LCD_DrawFillRectangle(15, 15, 35, 250, background_color);
            tft_draw_pointer();
        } else {
            printf("Failed to enter directory.\n");
        }
    }
    else {
        open_file();
        LCD_DrawFillRectangle(15, 15, 35, 250, background_color);
        LCD_Circle(25, selected_item*spacing + margin + 8, 8, true, GREEN);
    }
}

void tft_go_back_directory(void) {
    if (root_depth == 0) {
        // printf("Already at root directory.\n");
        return;
    }

    FRESULT fr = f_chdir("..");
    if (fr != FR_OK) {
        // printf("Failed to go up a directory: %d\n", fr);
        return;
    }

    root_depth--;

    list_directory();
    tft_draw_item_list();
    hovered_item = 0;
    LCD_DrawFillRectangle(15, 15, 35, 250, background_color);
    tft_draw_pointer();
}

void rotary_encoder_select(){
    if(gpio_get_irq_event_mask(ENC_A) & GPIO_IRQ_EDGE_RISE){
        tft_move_cursor(false);
        gpio_acknowledge_irq(ENC_A, GPIO_IRQ_EDGE_RISE);
    } else if(gpio_get_irq_event_mask(ENC_B) & GPIO_IRQ_EDGE_RISE){ 
        tft_move_cursor(true);
        gpio_acknowledge_irq(ENC_B, GPIO_IRQ_EDGE_RISE);
    } else if(gpio_get_irq_event_mask(ENC_SW) & GPIO_IRQ_EDGE_RISE){
        tft_select_item();
        gpio_acknowledge_irq(ENC_SW, GPIO_IRQ_EDGE_RISE);
    }
    if (gpio_get_irq_event_mask(BACK_PIN) & GPIO_IRQ_EDGE_RISE){
        tft_go_back_directory();
        gpio_acknowledge_irq(BACK_PIN, GPIO_IRQ_EDGE_RISE);
    }
}

void init_rotary_encoder(){
    gpio_set_function(ENC_A, GPIO_FUNC_SIO);
    gpio_set_function(ENC_B, GPIO_FUNC_SIO);
    gpio_set_function(ENC_SW, GPIO_FUNC_SIO);

    gpio_set_dir(ENC_A, GPIO_IN);
    gpio_set_dir(ENC_B, GPIO_IN);
    gpio_set_dir(ENC_SW, GPIO_IN);

    gpio_add_raw_irq_handler_masked((0x7<<2), rotary_encoder_select); // this will probably change based on what pins are used
    for (int i = 2; i <= 4; i++){
        gpio_set_irq_enabled(i, GPIO_IRQ_EDGE_RISE, true);
    }
    irq_set_enabled(IO_IRQ_BANK0, true);
}

void init_back_pin(){
    gpio_set_function(BACK_PIN, GPIO_FUNC_SIO);

    gpio_set_dir(BACK_PIN, GPIO_IN);

    gpio_add_raw_irq_handler(BACK_PIN, rotary_encoder_select);
    gpio_set_irq_enabled(BACK_PIN, GPIO_IRQ_EDGE_RISE, true);
}

void tft_init(){
    list_directory();
    tft_draw_item_list();
    LCD_DrawFillTriangle(200, margin + hovered_item*spacing, 200, margin + 16 + hovered_item*spacing, 180, margin + 8 + hovered_item*spacing, RED);
    LCD_DrawFillRectangle(0, 0, 10, 400, GRAYBLUE);
    LCD_DrawFillRectangle(0, 0, 240, 10, GRAYBLUE);
    LCD_DrawFillRectangle(230, 0, 240, 400, GRAYBLUE);
    LCD_DrawFillRectangle(0, 310, 240, 320, GRAYBLUE);
}


// JUST FOR REFERENCE: I wrote this in an isolated environment within one of the example labs (copied most of the functions into here), and this was the main I used as a reference

// int main() {
//     stdio_init_all();
    
//     init_sdcard_io();
    
//     init_spi_lcd();
//     init_back_pin();

//     keypad_init_pins(); <- this is just cuz i used the keypad as gpio pins
//     init_rotary_encoder();

//     LCD_Setup();
//     LCD_Clear(background_color);

//     mount_sd();

//     tft_init();

//     for(;;);
// }


// ***Pin config I used:***
// #define SD_MISO 12
// #define SD_CS 13
// #define SD_SCK 14
// #define SD_MOSI 15

// // display pins
// #define DISPLAY_SCK 18
// #define DISPLAY_TX 19
// #define DISPLAY_CSn 17
// #define DISPLAY_RST 21
// #define DISPLAY_DC 20

// // button pins
// #define BACK_PIN 5

// // rotary encoder pins
// #define ENC_A 2
// #define ENC_B 3
// #define ENC_SW 4
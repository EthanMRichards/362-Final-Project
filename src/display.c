#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "lcd.h"
#include <stdio.h>
#include <string.h>
#include <math.h>   
#include "images.h"
#include "initializations.h"

void rotary_encoder_select();

void init_spi_lcd_sd(){
    // SPI INITS for both SD and LCD
    // initialize SPI1 with 48 MHz clock
    spi_init(spi1, 400000); 
    spi_set_format(spi1, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_set_function(SD_RX, GPIO_FUNC_SPI);
    gpio_set_function(SD_TX, GPIO_FUNC_SPI);
    gpio_set_function(SD_SCK, GPIO_FUNC_SPI);

    // Chip Select Pins
    gpio_set_function(SD_CSn, GPIO_FUNC_SPI);
    gpio_set_function(DISPLAY_CSn, GPIO_FUNC_SPI);

    // OTHER GPIO INITS
    // DISPLAY
    gpio_set_function(DISPLAY_DC, GPIO_FUNC_SIO);
    gpio_set_function(DISPLAY_RST, GPIO_FUNC_SIO);

    gpio_set_dir(DISPLAY_DC, GPIO_OUT);
    gpio_set_dir(DISPLAY_RST, GPIO_OUT);

    gpio_put(DISPLAY_DC, 0); // DC low
    gpio_put(DISPLAY_RST, 1); // nRESET high

    // ROTARY ENCODER
    gpio_set_function(ENC_A, GPIO_FUNC_SIO);
    gpio_set_function(ENC_B, GPIO_FUNC_SIO);
    gpio_set_function(ENC_SW, GPIO_FUNC_SIO);

    gpio_set_dir(ENC_A, GPIO_OUT);
    gpio_set_dir(ENC_B, GPIO_OUT);
    gpio_set_dir(ENC_SW, GPIO_OUT);

    // Interrupt Initialization
    irq_set_exclusive_handler(IO_IRQ_BANK0, rotary_encoder_select);
    irq_set_enabled(IO_IRQ_BANK0, true);
}

Picture* load_image(const char* image_data);
void free_image(Picture* pic);

// The UI will list all the songs and have a cursor. 
// It will update the cursor and play a selected song based off the rotary encoder

// Initialize UI
void tft_init(){

}

void rotary_encoder_select(){
    if(gpio_get_irq_event_mask(ENC_A) & GPIO_IRQ_EDGE_RISE){
        tft_move_cursor(false);
        gpio_acknowledge_irq(ENC_A, GPIO_IRQ_EDGE_RISE);
    } else if(gpio_get_irq_event_mask(ENC_B) & GPIO_IRQ_EDGE_RISE){ 
        tft_move_cursor(true);
        gpio_acknowledge_irq(ENC_B, GPIO_IRQ_EDGE_RISE);
    } else if(gpio_get_irq_event_mask(ENC_SW) & GPIO_IRQ_EDGE_RISE){
        tft_select_song();
        gpio_acknowledge_irq(ENC_SW, GPIO_IRQ_EDGE_RISE);
    }
}

// Move Cursor Functions
void tft_move_cursor(bool right){ 
    // Check if curser will scroll past the first/last song if so return and don't update potision
    // Otherwise update position
}

void tft_select_song(){ 
    // Self explainatory, start playing new song if its hovering over it
}
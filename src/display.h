#ifndef DISPLAY_H
#define DISPLAY_H

void init_spi_lcd();

// ILI9341 Commands
#define ILI9341_SWRESET  0x01
#define ILI9341_SLPOUT   0x11
#define ILI9341_DISPOFF  0x28
#define ILI9341_DISPON   0x29
#define ILI9341_CASET    0x2A
#define ILI9341_PASET    0x2B
#define ILI9341_RAMWR    0x2C
#define ILI9341_MADCTL   0x36
#define ILI9341_COLMOD   0x3A
#define ILI9341_FRMCTR1  0xB1
#define ILI9341_DFUNCTR  0xB6
#define ILI9341_PWCTR1   0xC0
#define ILI9341_PWCTR2   0xC1
#define ILI9341_VMCTR1   0xC5
#define ILI9341_VMCTR2   0xC7
#define ILI9341_GMCTRP1  0xE0
#define ILI9341_GMCTRN1  0xE1

// Common colors (RGB565 format)
#define TFT_BLACK   0x0000
#define TFT_WHITE   0xFFFF
#define TFT_RED     0xF800
#define TFT_GREEN   0x07E0
#define TFT_BLUE    0x001F
#define TFT_YELLOW  0xFFE0
#define TFT_CYAN    0x07FF
#define TFT_MAGENTA 0xF81F

#endif
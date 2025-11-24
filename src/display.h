#ifndef DISPLAY_H
#define DISPLAY_H

void init_spi_tft();

// ILI9341 (TFT) Commands

#define TFTCMD_SWRST    0x01    // Software Reset
#define TFTCMD_SLPOUT   0x11    // Sleep-Mode Off
#define TFTCMD_DISPOFF  0x28    // Display Off
#define TFTCMD_DISPON   0x29    // Display On
#define TFTCMD_CASET    0x2A    // Define area of frame memory for MCU access through the values of SC [15:0] and EC [15:0]
#define TFTCMD_PASET    0x2B    // Define area of frame memory for MCU access through the values of SP [15:0] and EP [15:0]
#define TFTCMD_RAMWR    0x2C    // Transfer data from MCU to frame memory
#define TFTCMD_MADCTL   0x36    // Defines read/write scanning direction of frame memory
#define TFTCMD_COLMOD   0x3A    // Sets pixel format for RGB image data used by interface
#define TFTCMD_FRMCTR   0xB1    // PARAM_1 bit[1:0] DIVA : division ratio for internal clocks when Normal mode. PARAM_2 bit[4:0] RTNA : is used to set 1H (line) period of Normal mode at MCU interface
#define TFTCMD_DFUNCTR  0xB6    // Access to display functon control register, refer to documentation for specifics
#define TFTCMD_PWCTR1   0xC0    // VRH [5:0]: Set the GVDD level, which is a reference level for the VCOM level and the grayscale voltage level
#define TFTCMD_PWCTR2   0xC1    // BT [2:0]: Sets the factor used in the step-up circuits. Select the optimal step-up factor for the operating voltage. To reduce power consumption, set a smaller factor
#define TFTCMD_VMCTR1   0xC5    // PARAM_1 bit[6:0] VMH : Set the VCOMH voltage. PARAM_2 bit[6:0] VML : Set the VCOML voltage
#define TFTCMD_VMCTR2   0xC7    // nVM bit[7] : nVM equals to �0� after power on reset and VCOM offset equals to program MTP value. When nVM set to �1�, setting of VMF [6:0] becomes valid and VCOMH/VCOML can be adjusted. VMF [6:0]: Set the VCOM offset voltage
#define TFTCMD_GMCTRPOS 0xE0    // Set the gray scale voltage to adjust the gamma characteristics of the TFT panel
#define TFTCMD_GMCTRNEG 0xE1    // Set the gray scale voltage to adjust the gamma characteristics of the TFT panel.

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
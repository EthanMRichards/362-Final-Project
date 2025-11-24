// dac pins
#define DAC_SCK 6
#define DAC_TX 7
#define DAC_LR 47
#define DAC_SDA 16
#define DAC_SCL 9
#define DAC_RST 17

// sd pins
#define SD_SCK 14
#define SD_TX 15
#define SD_RX 12
#define SD_CSn 13
// will be changed as display pins will likely be on opposite side of the chip from sd pins
#define DISPLAY_SCK 42
#define DISPLAY_TX 43
#define DISPLAY_CSn 46
#define DISPLAY_RST 45
#define DISPLAY_DC 44

// button pins
#define BACK_PIN 39

// rotary encoder pins
#define ENC_A 37
#define ENC_B 38
#define ENC_SW 40

// EEPROM pins

#define EE_SCL 19
#define EE_SDA 18

void timer_isr();
void timer_init();
void spi_dac_init();
void pwm_dac_init();
void i2c_dac_init();
void spi_inter_init();
void gpio_pins_init();
void dma_init();
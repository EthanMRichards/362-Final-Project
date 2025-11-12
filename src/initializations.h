// dac pins
#define DAC_SCK 2
#define DAC_TX 3
#define DAC_LR 1
#define DAC_SDA 4
#define DAC_SCL 5
// spi 1 pins
#define SD_SCK 10
#define SD_TX 11
#define SD_RX 12
#define SD_CSn 13
// will be changed as display pins will likely be on opposite side of the chip from sd pins
#define DISPLAY_SCK 10
#define DISPLAY_TX 11
#define DISPLAY_CSn 14

// button pins
#define SELECT_PIN 15
#define BACK_PIN 16

// rotary encoder pins
#define ENC_A 17
#define ENC_B 18
#define ENC_SW 19

void timer_isr();
void timer_init();
void spi_dac_init();
void pwm_dac_init();
void i2c_dac_init();
void spi_inter_init();
void gpio_pins_init();
void dma_init();
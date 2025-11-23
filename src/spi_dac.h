void init_dac();
void pwm_dac_isr();

// Ideally always have at least a few parts of the data at a time
int16_t data_buffer[4] __attribute__((aligned(8)));

// FILLER GPIO NUMBERS WILL BE UPDATED LATER
#define DAC_MCLK 6
#define DAC_BCK 7


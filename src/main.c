    #include "headers.h"
    #include <stdio.h>
    #include <math.h>
    #include <stdlib.h>
    #include <string.h>
    #include "pico/stdlib.h"
    #include "hardware/uart.h"
    #include "biquad.h" 
    #include "initializations.h"
    #include "audio.h"
    #include "hardware/adc.h"
    #include "hardware/pwm.h"
    #include "queue.h"
    #include "support.h"
    #include "volume.h"

    // Audio output may be able to be done with the dma if we set up baud rate properly in the spi
    // The idea being that data will be sent at the exact right time by the spi, and then as the buffer empties the dma can be triggered to send more, saving cpu

    // use the first core to handle all interfacing excluding audio outputs, it will prioritize sd inputs, then user inputs, then gui outputs
    // the second core will be saved to do all of the audio processing and will do the audio outputs

    // will have 2 push buttons for select, back
    // rotary encoder can do the rest

    // The final gain will be done off chip to improve fidelity and reduce clipping

    // This is the one thing where we could probably use a potentiometer for gain and then an adc to just display the current volume
    // However if possible I would like to explore the digital options (keep prior idea the back pocket just in case)

    // If digital we could probably get away with 2 ^ 5 levels as this is more than typical already
    // From there we could probably use a resistor ladder dac for this into a voltage controlled amplifier
    // Depending on how the DAC works (I need to read its datasheet more) this could also fill in for this purpose

    // the audio outputs may need to be moved to the first core depending on resource usage
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Main
    // Start with pin initializations
    // Initialize the second core
    // will mostly be calling more information from the sd card so that it is constantly accessible to the second core
    // Most of the heavy lifting will be done by timers anyway
 int main(void) {
    stdio_init_all();
    init_pwm_audio(); 
    set_freq(0, 250.0f);
    set_freq(1, 0.0f);

    eq7_init(&g_eq);

    float gain_db = 0.0f;

    while (1) {
        gain_db = gain_tester(gain_db);
        eq7_set_gain(&g_eq, 1, gain_db);
        printf("Band 3 gain = %.2f dB\n", g_eq.gaindB[2]);
        sleep_ms(50);
    }
}

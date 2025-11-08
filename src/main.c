#include "interfacing.h"
#include "biquad.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

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


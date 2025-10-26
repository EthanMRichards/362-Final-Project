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

// Gain will be done off chip to improve fidelity

// the audio outputs may need to be moved to the first core depending on resource usage
//////////////////////////////////////////////////////////////////////////////

// initialization functions

// timer interrupts for outputs

// qspi for additional ram and flash (this may be unnecessary, look at datasheet closer)

// spi for data chip

// spi for sd card

// gpio for rotary encoder and other push buttons

// dma to take data from the sd card and put it into ram to be accessed efficiently
// another to efficiently take from ram and put it into cache (potentially) to save processing time (come back to this)

//////////////////////////////////////////////////////////////////////////////

// Audio Processing function(s)
// try to do a convolution over the each frame as determined in the file header
// if this is not fast enough, we may have to consider other options but this could be an incredibly clean output if it is fast enough

// to do this, simply multiply all coefficients with the values from the sd and sum them
// this will have to be run once on each channel (could be a huge bottleneck which is why we might need to consider other options)

//////////////////////////////////////////////////////////////////////////////

// Processing Coefficients generation function(s)
// This will be done on the first core, it is fine if there is some latency given the number of interrupts

// This will sum several exponentials to get a simulated frequency response curve in dt domain

//////////////////////////////////////////////////////////////////////////////

// Data control function to navigate sd card

//////////////////////////////////////////////////////////////////////////////

// IO control function
// Will get gain, produce commands for menu and sd navigation, as well as attain data for coefficients


//////////////////////////////////////////////////////////////////////////////

// GUI control function
// Will produce output for tft display.


//////////////////////////////////////////////////////////////////////////////

// Main



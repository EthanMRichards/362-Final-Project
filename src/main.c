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

// will have 4 push buttons for select, back, and then volume up and down

// rotary encoder can do the rest

// The final gain will be done off chip to improve fidelity and reduce clipping

// This is the one thing where we could probably use a potentiometer for gain and then an adc to just display the current volume
// However if possible I would like to explore the digital options (keep prior idea the back pocket just in case)

// If digital we could probably get away with 2 ^ 5 levels as this is more than typical already
// From there we could probably use a resistor ladder dac for this into a voltage controlled amplifier
// Depending on how the DAC works (I need to read its datasheet more) this could also fill in for this purpose

// the audio outputs may need to be moved to the first core depending on resource usage

//////////////////////////////////////////////////////////////////////////////

// initialization functions

// timer interrupts for outputs

// qspi for additional ram and flash (questionably necessary, look at datasheet closer)

// spi for dac chip

// spi for sd card and tft

// gpio for rotary encoder and other push buttons

// dma to take data from the sd card and put it into ram to be accessed efficiently
// another to efficiently take from ram and put it into cache (potentially) to save processing time (come back to this)



//////////////////////////////////////////////////////////////////////////////

// Audio Processing function(s)

// try to do a convolution over the each frame as determined in the file header
// if this is not fast enough, we may have to consider other options but this could be an incredibly clean output if it is fast enough

// to do this, simply "convolve" all coefficients with the values from the sd and sum them
// this will have to be run once on each channel (could be a huge bottleneck which is why we might need to consider other options)

// This function will also act as the main function for core two (tentative)



//////////////////////////////////////////////////////////////////////////////

// Processing Coefficients generation function(s)

// For this one we will have an already predetermined set of three functions which will correspond to a 1 gain over all frequencies
// These three functions can then just be multiplied by the determined coefficients and added together to get a function to convolve with the signal

// This will be done on the first core, it is fine if there is quite a bit of latency (a second or two) between user inputs and 
// a change given the number of interrupts handled by this core

// These functions will be causual, which is fine as all values are defined for audio playback, however this will help for the edge case at the beginning



//////////////////////////////////////////////////////////////////////////////

// Data control function to navigate sd card

// We can determine if it is feasable to choose individual songs, but right now, the goal is to choose individual songs within an album
// and play through them sequentially in the order of the album, and then go to the next song in the next album sequentially as well
// the scope of this problem can be decreased if necessary



//////////////////////////////////////////////////////////////////////////////

// IO control function

// Will get gain, produce commands for menus and sd navigation, as well as attain data for coefficients



//////////////////////////////////////////////////////////////////////////////

// GUI control function(s)
// Will produce output for tft display.

// We will have images which the rp2350 can pull from flash memory as "backgrounds" for the menus
// These will act as templates to get modified or stacked in the software before the output is pushed to the tft
// There will be 3 pages to navigate between, two separate menus (the third page being the intermediate between the two menus)
// The first menu will consist of a file explorer to get through different albums and songs on the sd card
// The second menu will be the eq menu which will display where the bands are currently and different adjustment levels
// The third page can also display the current song and where it is at
// For the sake of time, memory, and simplicity, album art will not be displayed, but we can come back to this should we desire to and have the time to



//////////////////////////////////////////////////////////////////////////////

// Main
// Start with pin initializations
// Initialize the second core
// will mostly be calling more information from the sd card so that it is constantly accessible to the second core
// Most of the heavy lifting will be done by timers anyway


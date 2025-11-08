#include "audio.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

void push(int left_data, int right_data){
    audio_data temp = current;
    while (temp.next != NULL){
        temp = *temp.next;
    }
    temp.next = malloc(sizeof(audio_data));
    (*temp.next).left = left_data;
    (*temp.next).right = right_data;
}

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

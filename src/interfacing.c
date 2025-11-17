#include "interfacing.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"


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



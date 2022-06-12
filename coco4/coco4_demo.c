// - video showing 8x16 color characters, with smooth scrolling in horizontal and vertical directions
// - audio playing simple tones
// - reading data from the SD card, possibly updating video buffer with this data



// planned clock speed: 201.4 MHz, exactly 8x the 25.175 MHz VGA pixel clock

// possible architecture: 2 RP2040's speaking over a quad SPI link
// one could handle either video & audio, or maybe video & audio & keyboard?
// the other would have I2C and SPI buses -- but for now, we will try and
// squeeze the whole thing onto a single RP2040

#include <coco4/coco4.h>
#include <stdio.h>
#include "pico/stdlib.h"



int main() {
    boot_system();
}

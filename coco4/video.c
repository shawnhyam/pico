#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/scanvideo.h"
#include "pico/scanvideo/composable_scanline.h"

void core1_func() {
    // the video should be a bit programmable; i.e. being able to specify what video modes to use
    // on which scanlines

    while (1) {

        // scanvideo_scanline_buffer_t *scanline_buffer = scanvideo_begin_scanline_generation(true);
        // draw_scanline_fragment(scanline_buffer);
        // scanvideo_end_scanline_generation(scanline_buffer);


        // can we trigger some work after the last scanline of a frame?
        // what is the best way to trigger work on core 1 during VSYNC? what work would we perform?
        // updating the frame data makes sense...
    }

}


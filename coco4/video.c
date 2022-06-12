#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/scanvideo.h"
#include "pico/scanvideo/composable_scanline.h"

// TODOs
// - mimic the display modes of the Motorola 6847... but possibly at 640x480 instead of 256x192
//      - so 3200 bytes instead of 512 bytes of display memory
//      - totally fixed character set (at least for now)
// - recreate the boot screen on the CoCo
//


static semaphore_t video_initted;

void core1_func() {
    // TODO the video should be a bit programmable; i.e. being able to specify what video modes to use
    // on which scanlines

    // initialize video and interrupts on core 1
    scanvideo_setup(&vga_mode_640x480_60);
    scanvideo_timing_enable(true);
    sem_release(&video_initted);


    while (1) {

        scanvideo_scanline_buffer_t *scanline_buffer = scanvideo_begin_scanline_generation(true);
        // draw_scanline_fragment(scanline_buffer);
        scanvideo_end_scanline_generation(scanline_buffer);


        // can we trigger some work after the last scanline of a frame?
        // what is the best way to trigger work on core 1 during VSYNC? what work would we perform?
        // updating the frame data makes sense...

        // could potentially change video modes, etc.
    }

}

void init_video() {
    // create a semaphore to be posted when video init is complete
    sem_init(&video_initted, 0, 1);

    // launch all the video on core 1, so it isn't affected by USB handling on core 0
    multicore_launch_core1(core1_func);

    // wait for initialization of video to be complete
    sem_acquire_blocking(&video_initted);
}


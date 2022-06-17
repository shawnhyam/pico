#include <stdio.h>

#include "pico/stdlib.h"
#include "vga/vga.h"

int main(void) {
    set_sys_clock_201_6mhz();

    // intialize stdio, send message so we can verify stdio + clock rates
    stdio_init_all();
    measure_freqs();

    // launch all the video on core 1, so it isn't affected by USB handling on
    // core 0
    multicore_launch_core1(modex_loop);

    // wait for initialization of video to be complete
    sem_acquire_blocking(&video_initted);

    return 0;
}

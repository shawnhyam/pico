#include <stdio.h>
#include "pico/stdlib.h"
#include "coco4/coco4.h"

void boot_system() {
    // FIXME
    set_sys_clock_48mhz();
    //set_sys_clock_khz(120000, false);

    stdio_init_all();

    //init_video();
    
    audio_test();
}
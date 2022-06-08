#include <stdio.h>

#include "hardware/clocks.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/scanvideo.h"
#include "pico/scanvideo/composable_scanline.h"

static semaphore_t video_initted;

// Palette
// There is a palette of 256 entries, each entry contains a 16-bit color
// memory = 512B

// Might be better to have Layer 1 as bitmap, and Layer 2 as tiles


// Layer 1: TILES / CHARACTERS
// Layer 1a: CHARACTER MODE
// 80x30 grid of tiles, each tile can be one of 256 patterns, each pattern is 8x16@1bpp
// 80x30 grid of colors, each 8-bit color entry contains a 4-bit index into palette for fg and bg
// tile map = 2400B, pattern map = 256*16 = 4kB
// consider defining an xoffset for each scanline, and wraparound behaviour... maybe a yoffset for each column?
// consider 80x40 grid instead, 8x12 @ 1bpp

// location of tile data; hoffset, voffset in pixels (position of 0,0)
// horizontal stride; number of bytes to increment when going to the next line
// height; determines wraparound???
// could also determine visible region to draw, a subset of the 80x30 (or 80x40) region
// i.e. hstride 128; height 64 (rows of 16 pixels); hoffset 13 (pixels); voffset 29 (pixels); all uint8_t ??
// i.e. hstride 80; height 40; hoffset 0; voffset 0
// wraps around in both horizontal & vertical directions
// can set the pixel height of each row? 12-16? (12 might be a squeeze...) would have to include info about where to slice the font data



// goal: read SD card, play tones, and update display all at the same time
// 




// Layer 1b: TILE MODE
// 40x30 grid of tiles, each tile can be one of 256 patterns, each pattern is 16x16@4bpp??
// tile map = 1200B, tile colors = ???, pattern map = 256*128 = 32kB


// Layer 2: BITMAP
// Want to be either 320x240@8bpp, or 640x480@2bpp...
// memory = 75kB
// could also be more "flexible" here... being able to define the start memory address for each scanline,
// and maybe the behaviour for wrapping around... could default to show a normal image but you could mess with it
// 

const scanvideo_timing_t my_vga_mode_640x480_60 =
        {
                .clock_freq = 25200000,

                .h_active = 640,
                .v_active = 480,

                .h_front_porch = 16,
                .h_pulse = 64,
                .h_total = 800,
                .h_sync_polarity = 1,

                .v_front_porch = 1,
                .v_pulse = 2,
                .v_total = 525,
                .v_sync_polarity = 1,

                .enable_clock = 0,
                .clock_polarity = 0,

                .enable_den = 0
        };

const scanvideo_mode_t my_mode = {
    .default_timing = &my_vga_mode_640x480_60,
    .pio_program = &video_24mhz_composable,
    .width = 640,
    .height = 480,
    .xscale = 1,
    .yscale = 1,
    .yscale_denominator = 1
};


// TODO move this into a helper library?
void measure_freqs(void) {
    uint f_pll_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
    uint f_pll_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_USB_CLKSRC_PRIMARY);
    uint f_rosc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_ROSC_CLKSRC);
    uint f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
    uint f_clk_peri = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI);
    uint f_clk_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB);
    uint f_clk_adc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_ADC);
    uint f_clk_rtc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC);
    uint sys_clk = clock_get_hz(clk_sys);

    clock_set_reported_hz(clk_sys, sys_clk);

 
    printf("pll_sys  = %dkHz\n", f_pll_sys);
    printf("pll_usb  = %dkHz\n", f_pll_usb);
    printf("rosc     = %dkHz\n", f_rosc);
    printf("clk_sys  = %dkHz\n", f_clk_sys);
    printf("clk_peri = %dkHz\n", f_clk_peri);
    printf("clk_usb  = %dkHz\n", f_clk_usb);
    printf("clk_adc  = %dkHz\n", f_clk_adc);
    printf("clk_rtc  = %dkHz\n", f_clk_rtc); 
    printf("sys_clk  = %dHz\n\n", sys_clk); 
}

uint16_t patterns[4][640];

void draw_scanline_fragment(scanvideo_scanline_buffer_t *buffer) {
    // figure out 1/32 of the color value
    uint line_num = scanvideo_scanline_number(buffer->scanline_id);
    uint frame_num = scanvideo_frame_number(buffer->scanline_id);

    // uint32_t *dst32 = buffer->data;
    // *dst32++ = COMPOSABLE_COLOR_RUN | 0xffff;
    // *dst32++ = 637 | COMPOSABLE_RAW_1P_SKIP_ALIGN;
    // *dst32++ = 0 | COMPOSABLE_EOL_ALIGN;

{
    uint16_t *p = (uint16_t *)buffer->data;
    if (line_num == 0 || line_num == 479) {
        *p++ = COMPOSABLE_COLOR_RUN;
        *p++ = PICO_SCANVIDEO_PIXEL_FROM_RGB5(31, 0, 0);
        *p++ = 315;
    *p++ = COMPOSABLE_RAW_1P;
    *p++ = 0;
    *p++ = COMPOSABLE_EOL_ALIGN;
    // *p++ = 0;
       
    } else {
        *p++ = COMPOSABLE_RAW_1P;
        *p++ = PICO_SCANVIDEO_PIXEL_FROM_RGB5(31, 0, 0);
        *p++ = COMPOSABLE_COLOR_RUN;
        *p++ = PICO_SCANVIDEO_PIXEL_FROM_RGB5(3, 3, 3);
        *p++ = 315;
        *p++ = COMPOSABLE_RAW_2P;
        *p++ = PICO_SCANVIDEO_PIXEL_FROM_RGB5(31, 0, 0);
        *p++ = 0;
        *p++ = COMPOSABLE_EOL_SKIP_ALIGN;
        *p++ = 0xffff;
    }

    buffer->data_used = ((uint32_t *)p) - buffer->data;
}

    {
         uint16_t *p = (uint16_t *)buffer->data2;
    if (line_num == 16 || line_num == 463) {
        *p++ = COMPOSABLE_COLOR_RUN;
     *p++ = PICO_SCANVIDEO_ALPHA_MASK | PICO_SCANVIDEO_PIXEL_FROM_RGB5(0, 31, 0);
    *p++ = 637;
    *p++ = COMPOSABLE_RAW_1P;
    *p++ = 0;
    *p++ = COMPOSABLE_EOL_ALIGN;
    // *p++ = 0;
       
    } else {
        *p++ = COMPOSABLE_RAW_1P;
        *p++ = PICO_SCANVIDEO_ALPHA_MASK | PICO_SCANVIDEO_PIXEL_FROM_RGB8(255, 0, 0);
        *p++ = COMPOSABLE_COLOR_RUN;
        *p++ = PICO_SCANVIDEO_PIXEL_FROM_RGB8(0, 0, 0);
        *p++ = 635;
        *p++ = COMPOSABLE_RAW_2P;
        *p++ = PICO_SCANVIDEO_PIXEL_FROM_RGB8(255, 0, 0);
        *p++ = 0;
        *p++ = COMPOSABLE_EOL_SKIP_ALIGN;
        *p++ = 0xffff;
    }
    buffer->data2_used = ((uint32_t *)p) - buffer->data2;
    }

    {
         uint16_t *p = (uint16_t *)buffer->data3;
    if (line_num == 32 || line_num == 447) {
        *p++ = COMPOSABLE_COLOR_RUN;
     *p++ = PICO_SCANVIDEO_ALPHA_MASK | PICO_SCANVIDEO_PIXEL_FROM_RGB5(0, 0, 31);
    *p++ = 637;
    *p++ = COMPOSABLE_RAW_1P;
    *p++ = 0;
    *p++ = COMPOSABLE_EOL_ALIGN;
    // *p++ = 0;
       
    } else {
        *p++ = COMPOSABLE_RAW_1P;
        *p++ = PICO_SCANVIDEO_PIXEL_FROM_RGB8(255, 0, 0);
        *p++ = COMPOSABLE_COLOR_RUN;
        *p++ = PICO_SCANVIDEO_PIXEL_FROM_RGB8(0, 0, 0);
        *p++ = 635;
        *p++ = COMPOSABLE_RAW_2P;
        *p++ = PICO_SCANVIDEO_PIXEL_FROM_RGB8(255, 0, 0);
        *p++ = 0;
        *p++ = COMPOSABLE_EOL_SKIP_ALIGN;
        *p++ = 0xffff;
    }
    buffer->data3_used = ((uint32_t *)p) - buffer->data3;
    }


    buffer->status = SCANLINE_OK;
}

void core1_func() {
        uint sys_clk = clock_get_hz(clk_sys);

    // initialize video and interrupts on core 1
    scanvideo_setup(&my_mode);
        uint sys_clk2 = clock_get_hz(clk_sys);

    scanvideo_timing_enable(true);
    sem_release(&video_initted);

    // how can we support a configurable background color, behind layer 1?

    while (true) {
        scanvideo_scanline_buffer_t *scanline_buffer = scanvideo_begin_scanline_generation(true);
        draw_scanline_fragment(scanline_buffer);
        scanvideo_end_scanline_generation(scanline_buffer);


        // can we trigger some work after the last scanline of a frame?
        // what is the best way to trigger work on core 1 during VSYNC? what work would we perform?
        // updating the frame data makes sense...
    }
}


int main(void) {
    // scanvideo library wants specific clock rates (integer multiples of 25MHz?)
    // pico-sdk/src/rp2_common/hardware_clocks/scripts/vcocalc.py 200
    // Requested: 200.0 MHz
    // Achieved: 200.0 MHz
    // FBDIV: 100 (VCO = 1200 MHz)
    // PD1: 6
    // PD2: 1
    //set_sys_clock_pll(1200000000, 6, 1);

    // Requested: 201.4 MHz
    // Achieved: 201.6 MHz
    // FBDIV: 84 (VCO = 1008 MHz)
    // PD1: 5
    // PD2: 1
    set_sys_clock_pll(1008000000, 5, 1);


    // intialize stdio, send message so we can verify stdio + clock rates
    stdio_init_all();
    measure_freqs(); 

    // launch all the video on core 1, so it isn't affected by USB handling on core 0
    multicore_launch_core1(core1_func);

    // wait for initialization of video to be complete
    sem_acquire_blocking(&video_initted);

    return 0;
}

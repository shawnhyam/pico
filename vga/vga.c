
#include "vga_inc.h"

semaphore_t video_initted;

void set_sys_clock_201_6mhz(void) {
    // scanvideo library wants specific clock rates
    // pico-sdk/src/rp2_common/hardware_clocks/scripts/vcocalc.py 201.4

    // Requested: 201.4 MHz
    // Achieved: 201.6 MHz
    // FBDIV: 84 (VCO = 1008 MHz)
    // PD1: 5
    // PD2: 1
    set_sys_clock_pll(1008000000, 5, 1);
}

// Palette
// There is a palette of 256 entries, each entry contains a 16-bit color
// memory = 512B

// Might be better to have Layer 1 as bitmap, and Layer 2 as tiles

// Layer 1: TILES / CHARACTERS
// Layer 1a: CHARACTER MODE
// 80x30 grid of tiles, each tile can be one of 256 patterns, each pattern is
// 8x16@1bpp 80x30 grid of colors, each 8-bit color entry contains a 4-bit index
// into palette for fg and bg tile map = 2400B, pattern map = 256*16 = 4kB
// consider defining an xoffset for each scanline, and wraparound behaviour...
// maybe a yoffset for each column? consider 80x40 grid instead, 8x12 @ 1bpp

// location of tile data; hoffset, voffset in pixels (position of 0,0)
// horizontal stride; number of bytes to increment when going to the next line
// height; determines wraparound???
// could also determine visible region to draw, a subset of the 80x30 (or 80x40)
// region i.e. hstride 128; height 64 (rows of 16 pixels); hoffset 13 (pixels);
// voffset 29 (pixels); all uint8_t ?? i.e. hstride 80; height 40; hoffset 0;
// voffset 0 wraps around in both horizontal & vertical directions can set the
// pixel height of each row? 12-16? (12 might be a squeeze...) would have to
// include info about where to slice the font data

// goal: read SD card, play tones, and update display all at the same time
//

// Layer 1b: TILE MODE
// 40x30 grid of tiles, each tile can be one of 256 patterns, each pattern is
// 16x16@4bpp?? tile map = 1200B, tile colors = ???, pattern map = 256*128 =
// 32kB

// Layer 2: BITMAP
// Want to be either 320x240@8bpp, or 640x480@2bpp...
// memory = 75kB
// could also be more "flexible" here... being able to define the start memory
// address for each scanline, and maybe the behaviour for wrapping around...
// could default to show a normal image but you could mess with it
//

const scanvideo_timing_t my_vga_mode_640x480_60 = {

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

const scanvideo_mode_t vga_mode = {

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


#include "minimal_computer.h"

#include "hardware/regs/addressmap.h"
#include "hardware/regs/rosc.h"
#include "pico/scanvideo/composable_scanline.h"

uint8_t mem[65536];

uint32_t rnd(void) {
    int k, random = 0;
    volatile uint32_t* rnd_reg = (uint32_t*)(ROSC_BASE + ROSC_RANDOMBIT_OFFSET);

    for (k = 0; k < 32; k++) {
        random = random << 1;
        random = random + (0x00000001 & (*rnd_reg));
    }
    return random;
}

uint8_t read6502(uint16_t address) {
    // printf("Read  %04x :  %02x\n", address, mem[address]);
    if (address == 0xfe) {
        return rnd() % 256;
    }
    return mem[address];
}

void write6502(uint16_t address, uint8_t value) {
    // printf("Write %04x -> %02x\n", address, value);
    mem[address] = value;
}

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

const scanvideo_mode_t my_mode = {

    .default_timing = &my_vga_mode_640x480_60,
    .pio_program = &video_24mhz_composable,
    .width = 640,
    .height = 480,
    .xscale = 1,
    .yscale = 1,
    .yscale_denominator = 1  //
};

uint16_t palette[16] = {
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x00, 0x00),  // 000000
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x1d, 0x2b, 0x53),  // 1D2B53
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x7e, 0x25, 0x53),  // 7E2553
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x87, 0x51),  // 008751
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xab, 0x52, 0x36),  // AB5236
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x5f, 0x57, 0x4f),  // 5F574F
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xc2, 0xc3, 0xc7),  // C2C3C7
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xff, 0xf1, 0xe8),  // FFF1E8
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xff, 0x00, 0x4d),  // FF004D
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xff, 0xa3, 0x00),  // FFA300
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xff, 0xec, 0x27),  // FFEC27
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0xe4, 0x36),  // 00E436
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x29, 0xad, 0xff),  // 29ADFF
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x83, 0x76, 0x9c),  // 83769C
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xff, 0x77, 0xa8),  // FF77A8
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xff, 0xcc, 0xaa)   // FFCCAA
};

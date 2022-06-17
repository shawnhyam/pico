#include "vga_inc.h"

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

void draw_scanline_fragment(scanvideo_scanline_buffer_t *buffer) {
    // figure out 1/32 of the color value
    uint line_num = scanvideo_scanline_number(buffer->scanline_id);
    uint frame_num = scanvideo_frame_number(buffer->scanline_id);

    uint16_t *p = (uint16_t *)buffer->data;
    uint v = line_num / 15;

    *p++ = COMPOSABLE_COLOR_RUN;
    *p++ = palette[1];  // PICO_SCANVIDEO_PIXEL_FROM_RGB8(0, 0, 0);
    *p++ = 77;

    for (int x = 0; x < 32; x++) {
        uint color = modez_mem[x + v * 32];  // (x+v)%16

        *p++ = COMPOSABLE_COLOR_RUN;
        *p++ = palette[color % 16];
        *p++ = 12;
    }

    *p++ = COMPOSABLE_COLOR_RUN;
    *p++ = palette[1];  // PICO_SCANVIDEO_PIXEL_FROM_RGB8(0, 0, 0);
    *p++ = 77;

    *p++ = COMPOSABLE_RAW_1P;
    *p++ = 0;
    *p++ = COMPOSABLE_EOL_ALIGN;

    buffer->data_used = ((uint32_t *)p) - buffer->data;
    buffer->status = SCANLINE_OK;
}

void modez_loop() {
    // initialize video and interrupts on core 1
    scanvideo_setup(&vga_mode);

    scanvideo_timing_enable(true);
    sem_release(&video_initted);

    // how can we support a configurable background color, behind layer 1?

    while (true) {
        scanvideo_scanline_buffer_t *scanline_buffer = scanvideo_begin_scanline_generation(true);
        draw_scanline_fragment(scanline_buffer);
        scanvideo_end_scanline_generation(scanline_buffer);

        // can we trigger some work after the last scanline of a frame?
        // what is the best way to trigger work on core 1 during VSYNC? what
        // work would we perform? updating the frame data makes sense...
    }
}

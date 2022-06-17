#include "vga_inc.h"

static uint32_t fgcolor32 = (PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xff, 0xff, 0xff) << 16) |
                            PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xff, 0xff, 0xff);

void mode0_draw_scanline_fragment(scanvideo_scanline_buffer_t *buffer) {
    uint line_num = scanvideo_scanline_number(buffer->scanline_id);
    uint frame_num = scanvideo_frame_number(buffer->scanline_id);

    uint row_num = line_num / 16;
    uint row_offset = (line_num % 16) / 2;
    uint8_t *vmem = mode0_vidmem + (row_num * 40);
    uint8_t *cmem = mode0_charmem + row_offset;

    uint16_t *p = (uint16_t *)buffer->data;
    *p++ = COMPOSABLE_RAW_RUN;
    *p++ = 0;

    uint32_t *dst32 = (uint32_t *)(p);
    for (int i = 0; i < 40; i++) {
        // get the character from video memory
        uint8_t ch = *vmem++;
        // look up the pattern from character memory (8 bytes per character, 1 per line)
        uint8_t pattern = cmem[ch << 3];

        *dst32++ = pattern & 128 ? fgcolor32 : 0;
        *dst32++ = pattern & 64 ? fgcolor32 : 0;
        *dst32++ = pattern & 32 ? fgcolor32 : 0;
        *dst32++ = pattern & 16 ? fgcolor32 : 0;
        *dst32++ = pattern & 8 ? fgcolor32 : 0;
        *dst32++ = pattern & 4 ? fgcolor32 : 0;
        *dst32++ = pattern & 2 ? fgcolor32 : 0;
        *dst32++ = pattern & 1 ? fgcolor32 : 0;
    }
    p = (uint16_t *)dst32;

    *p++ = COMPOSABLE_RAW_1P;
    *p++ = 0;
    *p++ = COMPOSABLE_EOL_ALIGN;

    buffer->data_used = ((uint32_t *)p) - buffer->data;
    // fix the pixel/run count issue back at the beginning
    p = (uint16_t *)buffer->data;
    p[1] = p[2];
    p[2] = 40 * 16 - 3;

    buffer->status = SCANLINE_OK;
}

void mode0_loop(void) {
    // initialize video and interrupts on core 1
    scanvideo_setup(&vga_mode);
    scanvideo_timing_enable(true);

    sem_release(&video_initted);

    while (true) {
        scanvideo_scanline_buffer_t *scanline_buffer = scanvideo_begin_scanline_generation(true);
        mode0_draw_scanline_fragment(scanline_buffer);
        scanvideo_end_scanline_generation(scanline_buffer);
    }
}
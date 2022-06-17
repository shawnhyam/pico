#include "vga_inc.h"

static uint32_t mask_table[256][4];

static uint32_t fgcolor32 = (PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xff, 0xff, 0xff) << 16) |
                            PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xff, 0xff, 0xff);

void mode1_draw_scanline_fragment(scanvideo_scanline_buffer_t *buffer) {
    uint line_num = scanvideo_scanline_number(buffer->scanline_id);
    uint frame_num = scanvideo_frame_number(buffer->scanline_id);

    uint row_num = line_num / 12;
    uint row_offset = line_num % 12;
    uint8_t *vmem = mode1_vidmem + (row_num * 80);
    uint8_t *cmem = mode1_charmem + row_offset;

    uint16_t *p = (uint16_t *)buffer->data;
    *p++ = COMPOSABLE_RAW_RUN;
    *p++ = 0;

    uint32_t *dst32 = (uint32_t *)(p);
    for (int i = 0; i < 80; i++) {
        // get the character from video memory
        uint8_t ch = *vmem++;
        // look up the pattern from character memory (12 bytes per character, 1 per line)
        uint8_t pattern = cmem[ch * 12];

        uint32_t *mask = mask_table[pattern];
        *dst32++ = fgcolor32 & *mask++;
        *dst32++ = fgcolor32 & *mask++;
        *dst32++ = fgcolor32 & *mask++;
        *dst32++ = fgcolor32 & *mask++;
    }
    p = (uint16_t *)dst32;

    *p++ = COMPOSABLE_RAW_1P;
    *p++ = 0;
    *p++ = COMPOSABLE_EOL_ALIGN;

    buffer->data_used = ((uint32_t *)p) - buffer->data;
    // fix the pixel/run count issue back at the beginning
    p = (uint16_t *)buffer->data;
    p[1] = p[2];
    p[2] = 80 * 8 - 3;

    buffer->status = SCANLINE_OK;
}

void mode1_loop(void) {
    uint8_t mask[8] = {1, 2, 4, 8, 16, 32, 64, 128};

    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 4; j++) {
            mask_table[i][j] = 0;
        }

        if (i & mask[7]) {
            mask_table[i][0] |= 0x0000ffff;
        }
        if (i & mask[6]) {
            mask_table[i][0] |= 0xffff0000;
        }
        if (i & mask[5]) {
            mask_table[i][1] |= 0x0000ffff;
        }
        if (i & mask[4]) {
            mask_table[i][1] |= 0xffff0000;
        }
        if (i & mask[3]) {
            mask_table[i][2] |= 0x0000ffff;
        }
        if (i & mask[2]) {
            mask_table[i][2] |= 0xffff0000;
        }
        if (i & mask[1]) {
            mask_table[i][3] |= 0x0000ffff;
        }
        if (i & mask[0]) {
            mask_table[i][3] |= 0xffff0000;
        }
    }

    // initialize video and interrupts on core 1
    scanvideo_setup(&vga_mode);
    scanvideo_timing_enable(true);

    sem_release(&video_initted);

    while (true) {
        scanvideo_scanline_buffer_t *scanline_buffer = scanvideo_begin_scanline_generation(true);
        mode1_draw_scanline_fragment(scanline_buffer);
        scanvideo_end_scanline_generation(scanline_buffer);
    }
}
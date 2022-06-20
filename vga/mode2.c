#include "vga_inc.h"

int screen_width = 32;
int screen_height = 16;

static uint32_t coco_bgcolor32 = (PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x01, 0xff, 0x00) << 16) |
                                 PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x01, 0xff, 0x00);
static uint32_t coco_fgcolor32 = (PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x40, 0x00) << 16) |
                                 PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x40, 0x00);

static uint32_t coco_semicolor32[8] = {PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x01, 0xff, 0x00),
                                       PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xff, 0xff, 0x00),
                                       PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x00, 0xff),
                                       PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xff, 0x00, 0x00),
                                       PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xff, 0xff, 0xff),
                                       PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x01, 0xff, 0xff),
                                       PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xff, 0x00, 0xff),
                                       PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xff, 0x80, 0x00)};

static void mode2_draw_scanline_fragment(scanvideo_scanline_buffer_t *buffer) {
    uint line_num = scanvideo_scanline_number(buffer->scanline_id);
    uint frame_num = scanvideo_frame_number(buffer->scanline_id);

    uint16_t *p = (uint16_t *)buffer->data;

    int vspace = (480 - (screen_height * 24)) / 2;
    if ((line_num < vspace) || (line_num >= 480 - vspace)) {
        // just draw black
        *p++ = COMPOSABLE_COLOR_RUN;
        *p++ = 0;  // black
        *p++ = 640 - 3;

        *p++ = COMPOSABLE_RAW_1P;
        *p++ = 0;  // black

        *p++ = COMPOSABLE_EOL_ALIGN;

        buffer->data_used = ((uint32_t *)p) - buffer->data;
        buffer->status = SCANLINE_OK;
        return;
    }

    int hspace = (640 - (screen_width * 16)) / 2;

    // do this in 2 blocks so the alignment doesn't get off
    *p++ = COMPOSABLE_COLOR_RUN;
    *p++ = 0;  // black
    *p++ = hspace - 6;

    *p++ = COMPOSABLE_COLOR_RUN;
    *p++ = 0;  // black
    *p++ = 0;

    uint row_num = (line_num - vspace) / 24;
    uint row_offset = ((line_num - vspace) % 24) / 2;
    bool upper_half = row_offset < 6;  // for semigraphics mode

    uint8_t *vmem = mode2_vidmem + (row_num * screen_width);
    uint8_t *cmem = mode2_charmem + row_offset;

    *p++ = COMPOSABLE_RAW_RUN;
    uint16_t *q = p;  // remember this position
    *p++ = 0;

    uint32_t *dst32 = (uint32_t *)(p);
    for (int i = 0; i < screen_width; i++) {
        // get the character from video memory
        uint8_t ch = *vmem++;

        if (ch < 32) {
            *dst32++ = coco_bgcolor32;
            *dst32++ = coco_bgcolor32;
            *dst32++ = coco_bgcolor32;
            *dst32++ = coco_bgcolor32;
            *dst32++ = coco_bgcolor32;
            *dst32++ = coco_bgcolor32;
            *dst32++ = coco_bgcolor32;
            *dst32++ = coco_bgcolor32;

        } else if (ch < 128) {
            // look up the pattern from character memory (12 bytes per character, 1 per line)
            uint8_t pattern = cmem[(ch - 32) * 12];

            *dst32++ = pattern & 128 ? coco_fgcolor32 : coco_bgcolor32;
            *dst32++ = pattern & 64 ? coco_fgcolor32 : coco_bgcolor32;
            *dst32++ = pattern & 32 ? coco_fgcolor32 : coco_bgcolor32;
            *dst32++ = pattern & 16 ? coco_fgcolor32 : coco_bgcolor32;
            *dst32++ = pattern & 8 ? coco_fgcolor32 : coco_bgcolor32;
            *dst32++ = pattern & 4 ? coco_fgcolor32 : coco_bgcolor32;
            *dst32++ = pattern & 2 ? coco_fgcolor32 : coco_bgcolor32;
            *dst32++ = pattern & 1 ? coco_fgcolor32 : coco_bgcolor32;
        } else {
            // semigraphics characters
            bool first = upper_half ? ch & 8 : ch & 2;
            bool second = upper_half ? ch & 4 : ch & 1;
            // TODO color is determined by 4 bits
            int color_idx = (ch & 0b01110000) >> 4;
            uint32_t color = coco_semicolor32[color_idx];
            *dst32++ = first ? color : 0;
            *dst32++ = first ? color : 0;
            *dst32++ = first ? color : 0;
            *dst32++ = first ? color : 0;
            *dst32++ = second ? color : 0;
            *dst32++ = second ? color : 0;
            *dst32++ = second ? color : 0;
            *dst32++ = second ? color : 0;
        }
    }
    p = (uint16_t *)dst32;

    *p++ = COMPOSABLE_COLOR_RUN;
    *p++ = 0;  // black
    *p++ = hspace - 3;

    *p++ = COMPOSABLE_EOL_ALIGN;

    buffer->data_used = ((uint32_t *)p) - buffer->data;
    // fix the pixel/run count issue back at the beginning
    q[0] = q[1];
    q[1] = screen_width * 16 - 3;

    buffer->status = SCANLINE_OK;
}

void mode2_loop(void) {
    for (int i = 0; i < 8; i++) {
        uint32_t tmp = coco_semicolor32[i];
        coco_semicolor32[i] = (tmp << 16) | tmp;
    }

    // initialize video and interrupts on core 1
    scanvideo_setup(&vga_mode);
    scanvideo_timing_enable(true);

    sem_release(&video_initted);

    while (true) {
        scanvideo_scanline_buffer_t *scanline_buffer = scanvideo_begin_scanline_generation(true);
        mode2_draw_scanline_fragment(scanline_buffer);
        scanvideo_end_scanline_generation(scanline_buffer);
    }
}
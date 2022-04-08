/*
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include "pico.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/scanvideo.h"
#include "pico/scanvideo/composable_scanline.h"
#include "pico/sync.h"
#include "AVGA2_8x16.h"

#define vga_mode vga_mode_640x480_60

typedef struct pattern32 {
    uint8_t p0;
    uint8_t p1;
    uint8_t p2;
    uint8_t p3;
} pattern32_t;

typedef struct scanline {
    uint32_t *data;
    uint16_t data_used;
} scanline_t;

typedef union {
    struct {
        uint8_t field: 5;
        uint8_t field2: 4;
        /* and so on... */
    } fields;
    uint32_t bits;
} some_struct_t;

typedef union {
    struct {
        uint8_t fg : 4;
        uint8_t bg : 4;
    } v;
    uint16_t bits;
 } color_t;

typedef struct mixedcolor {
    uint32_t fg32;
    uint32_t bg32;
} mixedcolor_t;

typedef struct screen_char {
    uint8_t glyph;
    color_t color;
} screen_char_t;

typedef struct screen_char32 {
    screen_char_t char0;
    screen_char_t char1;
} screen_char32_t;


static semaphore_t video_initted;

static uint8_t *font = AVGA2_8x16;
static screen_char_t screen[30][80];
static color_t colors[30][80];

static uint32_t mask_table[256][4];


static uint16_t palette[16] = {
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0, 0, 0),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(157, 157, 157),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(255, 255, 255),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(190, 38, 51),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(224, 111, 139),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(73, 60, 43),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(164, 100, 34),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(235, 137, 49),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(247, 226, 107),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(47, 72, 78),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(68, 137, 26),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(163, 206, 39),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(27, 38, 50),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0, 87, 132),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(49, 162, 242),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(178, 220, 239)
};

static uint32_t palette32[16];
static mixedcolor_t bigpalette[256];
static uint8_t pattern_buffer[480][80];

void expand_patterns(uint8_t *data, uint16_t line_num) {
    uint8_t *p = data;

    uint8_t *font_base = font + line_num%16;
    screen_char_t *screen_pos = screen[line_num/16];

    for (int i=0; i<80; i++) {
        screen_char_t screen_char = *screen_pos++;
        uint8_t glyph = screen_char.glyph;
        uint8_t pattern = font_base[glyph << 4];
        *p++ = pattern;
    }
}

void draw_init(void) {
    uint8_t mask[8] = { 1,2,4,8,16,32,64,128 };

    for (int i=0; i<256; i++) {
        for (int j=0; j<4; j++) {
            mask_table[i][j] = 0;
        }

        if (i & mask[7]) { mask_table[i][0] |= 0x0000ffff; }
        if (i & mask[6]) { mask_table[i][0] |= 0xffff0000; }
        if (i & mask[5]) { mask_table[i][1] |= 0x0000ffff; }
        if (i & mask[4]) { mask_table[i][1] |= 0xffff0000; }
        if (i & mask[3]) { mask_table[i][2] |= 0x0000ffff; }
        if (i & mask[2]) { mask_table[i][2] |= 0xffff0000; }
        if (i & mask[1]) { mask_table[i][3] |= 0x0000ffff; }
        if (i & mask[0]) { mask_table[i][3] |= 0xffff0000; }
    }

    for (int i=0; i<16; i++) {
        palette32[i] = (palette[i] << 16) | palette[i];
    }
    for (int i=0; i<256; i++) {
        bigpalette[i].fg32 = palette32[i%16];
        bigpalette[i].bg32 = palette32[i/16];
    }

    uint8_t c = 0;
    for (int y=0; y<30; y++) {
        for (int x=0; x<80; x++) {
            colors[y][x].v.bg = y%16;
            colors[y][x].v.fg = c%16;
            screen[y][x].color = colors[y][x];
            screen[y][x].glyph = c++;
        }
    }

    int total = 0;
    for (int line=0; line<480; line++) {
        expand_patterns(pattern_buffer[line], line);
    }
}

void draw_pattern(scanvideo_scanline_buffer_t *buffer) {
    // figure out 1/32 of the color value
    uint line_num = scanvideo_scanline_number(buffer->scanline_id);
    uint frame_num = scanvideo_frame_number(buffer->scanline_id);

    uint16_t *p = (uint16_t *) buffer->data;
    uint16_t *q = p+1;
    *p++ = COMPOSABLE_RAW_RUN;
    *p++ = 80*8 - 2;

    color_t *color_pos = colors[line_num/16];

    pattern32_t *src32 = (pattern32_t *)pattern_buffer[line_num];
    uint32_t *dst32 = (uint32_t *)p;

    for (int i=0; i<20; i++) {
        // grab a 32-bit pattern... that's 4 characters
        pattern32_t pattern = *src32++;

        // get the mask and colors for the first character
        uint32_t *mask = mask_table[pattern.p0];
        uint32_t maskval = *mask++;

        color_t color = *color_pos++;
        uint32_t fgcolor32 = palette32[color.v.fg];
        uint32_t bgcolor32 = palette32[color.v.bg];

        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);

        // second character
        mask = mask_table[pattern.p1];
        maskval = *mask++;

        color = *color_pos++;
        fgcolor32 = palette32[color.v.fg];
        bgcolor32 = palette32[color.v.bg];

        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);

        // third character
        mask = mask_table[pattern.p1];
        maskval = *mask++;

        color = *color_pos++;
        fgcolor32 = palette32[color.v.fg];
        bgcolor32 = palette32[color.v.bg];

        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);

        // fourth and final character
        mask = mask_table[pattern.p1];
        maskval = *mask++;

        color = *color_pos++;
        fgcolor32 = palette32[color.v.fg];
        bgcolor32 = palette32[color.v.bg];

        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
    }

        p = (uint16_t *)dst32;
    *p++ = 0x0000;  // one black pixel to end it

    // swap elements 1 & 2, due to the structure of a COMPOSABLE_RAW_RUN
    uint16_t tmp0 = *q++;
    uint16_t tmp1 = *q--;
    *q++ = tmp1;
    *q = tmp0;

    // end of line with alignment padding
    *p++ = COMPOSABLE_EOL_ALIGN;
    *p++ = 0;

    buffer->data_used = ((uint32_t *) p) - buffer->data;
    // assert(buffer->data_used < buffer->data_max);

    buffer->status = SCANLINE_OK;

}

void draw_scanline(scanvideo_scanline_buffer_t *buffer) {
    // figure out 1/32 of the color value
    uint line_num = scanvideo_scanline_number(buffer->scanline_id);
    uint frame_num = scanvideo_frame_number(buffer->scanline_id);

    uint16_t *p = (uint16_t *) buffer->data;
    uint16_t *q = p+1;
    *p++ = COMPOSABLE_RAW_RUN;
    *p++ = 80*8 - 2;

    uint8_t *font_base = font + line_num%16;
    uint32_t *dst32 = (uint32_t *)p;

#if 0
    screen_char32_t *screen_pos = (screen_char32_t *)screen[line_num/16];
    for (int i=0; i<40; i++) {
        // fetch memory for 2 characters (2 glyphs and 2 color pairs)
        screen_char32_t screen_char = *screen_pos++;

        // first character
        uint8_t glyph = screen_char.char0.glyph;
        uint8_t pattern = font_base[glyph << 4];

        mixedcolor_t color_pair = bigpalette[screen_char.char0.color.bits];
        // uint32_t fgcolor32 = palette32[screen_char.char0.color.fg];
        // uint32_t bgcolor32 = palette32[screen_char.char0.color.bg];
        uint32_t fgcolor32 = color_pair.fg32;
        uint32_t bgcolor32 = color_pair.bg32;

        uint32_t *mask = mask_table[pattern];
        uint32_t maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);

        // second character
        glyph = screen_char.char1.glyph;
        pattern = font_base[glyph << 4];

        fgcolor32 = palette32[screen_char.char1.color.v.fg];
        bgcolor32 = palette32[screen_char.char1.color.v.bg];

        mask = mask_table[pattern];
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
    }
#else
    screen_char_t *screen_pos = (screen_char_t *)screen[line_num/16];
    for (int i=0; i<80; i++) {
        screen_char_t screen_char = *screen_pos++;

        // first character
        uint8_t glyph = screen_char.glyph;
        uint8_t pattern = font_base[glyph << 4];

        uint32_t fgcolor32 = palette32[screen_char.color.v.fg];
        uint32_t bgcolor32 = palette32[screen_char.color.v.bg];

        uint32_t *mask = mask_table[pattern];
        uint32_t maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
    }
#endif
    p = (uint16_t *)dst32;
    *p++ = 0x0000;  // one black pixel to end it

    // swap elements 1 & 2, due to the structure of a COMPOSABLE_RAW_RUN
    uint16_t tmp0 = *q++;
    uint16_t tmp1 = *q--;
    *q++ = tmp1;
    *q = tmp0;

    // end of line with alignment padding
    *p++ = COMPOSABLE_EOL_ALIGN;
    *p++ = 0;

    buffer->data_used = ((uint32_t *) p) - buffer->data;
    // assert(buffer->data_used < buffer->data_max);

    buffer->status = SCANLINE_OK;
}



void core1_func() {

    // initialize video and interrupts on core 1
    scanvideo_setup(&vga_mode);
    scanvideo_timing_enable(true);
    sem_release(&video_initted);

    while (true) {
        scanvideo_scanline_buffer_t *scanline_buffer = scanvideo_begin_scanline_generation(true);
        draw_pattern(scanline_buffer);
        scanvideo_end_scanline_generation(scanline_buffer);
    }
}

int main(void) {
    set_sys_clock_khz(150000, true);

    draw_init();

    // create a semaphore to be posted when video init is complete
    sem_init(&video_initted, 0, 1);

    // launch all the video on core 1, so it isn't affected by USB handling on core 0
    multicore_launch_core1(core1_func);

    // wait for initialization of video to be complete
    sem_acquire_blocking(&video_initted);

    stdio_init_all();

    while (true) {
        // prevent tearing when we invert - if you're astute you'll notice this actually causes
        // a fixed tear a number of scanlines from the top. this is caused by pre-buffering of scanlines
        // and is too detailed a topic to fix here.
        scanvideo_wait_for_vblank();
        int c = getchar_timeout_us(0);
        switch (c) {
            case ' ':
                printf("Hello %d %d\n", sizeof(screen_char_t), sizeof(screen_char32_t));
                break;
        }
    }
}

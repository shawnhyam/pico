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
#include "foo.h"

#define vga_mode vga_mode_640x480_60

static semaphore_t video_initted;

static uint8_t *font = AVGA2_8x16;
static screen_char_t screen[30][80];
static color_t colors[30][80];

static uint32_t mask_table[256][4];


static __not_in_flash("x") uint16_t palette[16] = {
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


// 2 header + 641 pixels + 1 footer
uint16_t scanline[16][644];


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


    //
    for (int j=0; j<16; j++) {
        scanline[j][0] = COMPOSABLE_RAW_RUN;
        for (int i=0; i<640; i++) {
            uint16_t color = palette[(i+j)%16];
            scanline[j][i+2] = palette[color];
        }
    scanline[j][1] = scanline[j][2];
    scanline[j][2] = 638;
    scanline[j][642] = 0;
    scanline[j][643] = COMPOSABLE_EOL_ALIGN;


    }
}

void draw_scanline_baseline(scanvideo_scanline_buffer_t *buffer) {
    // figure out 1/32 of the color value
    uint line_num = scanvideo_scanline_number(buffer->scanline_id);
    uint frame_num = scanvideo_frame_number(buffer->scanline_id);

    uint16_t *p = (uint16_t *) buffer->data;
    uint16_t *q = p+1;
    *p++ = COMPOSABLE_RAW_RUN;
    *p++ = 80*8 - 2;

    uint16_t bgcolor = palette[(line_num/16) % 16];
    for (int i=0; i<640; i++) {
        *p++ = bgcolor;
    }

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

const uint32_t line_start = (638 << 16) | COMPOSABLE_RAW_RUN;
const uint32_t line_end = (COMPOSABLE_EOL_ALIGN << 16) | (0);

void draw_scanline_baseline32(scanvideo_scanline_buffer_t *buffer) {
    // figure out 1/32 of the color value
    uint line_num = scanvideo_scanline_number(buffer->scanline_id);
    uint frame_num = scanvideo_frame_number(buffer->scanline_id);

    uint32_t *dst32 = buffer->data;
    *dst32++ = line_start;

    uint32_t bgcolor32 = palette32[line_num % 16];

    for (int i=0; i<320; i++) {
        *dst32++ = bgcolor32;
    }
    *dst32++ = line_end;

    // swap elements 1 & 2, due to the structure of a COMPOSABLE_RAW_RUN
    uint16_t *q = (uint16_t *)buffer->data;
    uint16_t tmp = q[1];
    q[1] = q[2];
    q[2] = tmp;

    buffer->data_used = dst32 - buffer->data;
    buffer->status = SCANLINE_OK;
}

static int pos = 0;

void draw_scanline_fragment(scanvideo_scanline_buffer_t *buffer) {
    // figure out 1/32 of the color value
    uint line_num = scanvideo_scanline_number(buffer->scanline_id);
    uint frame_num = scanvideo_frame_number(buffer->scanline_id);
    uint idx = line_num%16;

    buffer->fragment_words = 322;
    uint32_t *dst32 = buffer->data;
    //scanline[idx][1] = palette[line_num%16];
    *dst32++ = host_safe_hw_ptr(scanline[idx]);

    buffer->data_used = dst32 - buffer->data;
    buffer->status = SCANLINE_OK;

    // scanline[(idx+1)%8][(pos++)%638+2] = 0;
    scanline[(idx+8)%16][(pos%630)+3] += 1;
    pos += 317;
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

static inline void draw_character(screen_char_t screen_char, int col, int line, uint16_t *buffers[4]) {
    uint8_t glyph = screen_char.glyph;
    uint32_t fgcolor32 = palette32[screen_char.color.v.fg];
    uint32_t bgcolor32 = palette32[screen_char.color.v.bg];

        pattern32_t *font_base = (pattern32_t *)(font + (glyph << 4) + line);
        pattern32_t pattern32 = *font_base;

        uint32_t *mask = mask_table[pattern32.p0];
        uint32_t maskval = *mask++;

        uint32_t *dst32 = (uint32_t *)(buffers[0] + (col << 3));
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32 = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);

        mask = mask_table[pattern32.p1];
        maskval = *mask++;

        dst32 = (uint32_t *)(buffers[1] + (col << 3));
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32 = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);

        mask = mask_table[pattern32.p2];
        maskval = *mask++;

        dst32 = (uint32_t *)(buffers[2] + (col << 3));
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32 = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);

        mask = mask_table[pattern32.p3];
        maskval = *mask++;

        dst32 = (uint32_t *)(buffers[3] + (col << 3));
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32 = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
}

static inline void draw_character32(screen_char32_t screen_char, int c, int line, uint16_t *buffers[4]) {
    int col = c<<1;
    uint8_t glyph = screen_char.char0.glyph;
    uint32_t fgcolor32 = palette32[screen_char.char0.color.v.fg];
    uint32_t bgcolor32 = palette32[screen_char.char0.color.v.bg];

        pattern32_t *font_base = (pattern32_t *)(font + (glyph << 4) + line);
        pattern32_t pattern32 = *font_base;

        uint32_t *mask = mask_table[pattern32.p0];
        uint32_t maskval = *mask++;

        uint32_t *dst32 = (uint32_t *)(buffers[0] + (col << 3));
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32 = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);

        mask = mask_table[pattern32.p1];
        maskval = *mask++;

        dst32 = (uint32_t *)(buffers[1] + (col << 3));
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32 = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);

        mask = mask_table[pattern32.p2];
        maskval = *mask++;

        dst32 = (uint32_t *)(buffers[2] + (col << 3));
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32 = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);

        mask = mask_table[pattern32.p3];
        maskval = *mask++;

        dst32 = (uint32_t *)(buffers[3] + (col << 3));
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32 = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);

    col++;

 glyph = screen_char.char1.glyph;
     fgcolor32 = palette32[screen_char.char1.color.v.fg];
     bgcolor32 = palette32[screen_char.char1.color.v.bg];

        font_base = (pattern32_t *)(font + (glyph << 4) + line);
     pattern32 = *font_base;

         mask = mask_table[pattern32.p0];
         maskval = *mask++;

         dst32 = (uint32_t *)(buffers[0] + (col << 3));
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32 = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);

        mask = mask_table[pattern32.p1];
        maskval = *mask++;

        dst32 = (uint32_t *)(buffers[1] + (col << 3));
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32 = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);

        mask = mask_table[pattern32.p2];
        maskval = *mask++;

        dst32 = (uint32_t *)(buffers[2] + (col << 3));
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32 = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);

        mask = mask_table[pattern32.p3];
        maskval = *mask++;

        dst32 = (uint32_t *)(buffers[3] + (col << 3));
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
        maskval = *mask++;
        *dst32 = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);

}


void draw_scanline(scanvideo_scanline_buffer_t *buffers[4]) {
    // figure out 1/32 of the color value
    uint master_line_num = scanvideo_scanline_number(buffers[0]->scanline_id);
    uint master_frame_num = scanvideo_frame_number(buffers[0]->scanline_id);

    uint16_t *ps[4], *qs[4];

    for (int j=0; j<4; j++) {
        ps[j] = (uint16_t *)buffers[j]->data;
    }

    for (int j=0; j<4; j++) {
        *ps[j]++ = COMPOSABLE_RAW_RUN;
        qs[j] = ps[j];
        *ps[j]++ = 80*8 - 2;
    }

    // uint8_t *font_base = font + line_num%16;
    // uint32_t *dst32 = (uint32_t *)p;

    screen_char32_t *screen_pos = (screen_char32_t *)screen[master_line_num/16];
    for (int i=0; i<40; i++) {
        screen_char32_t screen_char = *screen_pos++;

        // first character
        // uint8_t glyph = screen_char.glyph;
        // uint32_t fgcolor32 = palette32[screen_char.color.v.fg];
        // uint32_t bgcolor32 = palette32[screen_char.color.v.bg];

        draw_character32(screen_char, i, master_line_num%16, ps);

#if 0
        // for each line
        for (int j=0; j<1; j++) {
            uint8_t pattern = font_base[glyph << 4] + j;

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
    }

    for (int j=0; j<4; j++) {
        uint16_t *p = ps[j] + (80 << 3);
        uint16_t *q = qs[j];
    // p = (uint16_t *)dst32;
    *p++ = 0x0000;  // one black pixel to end it

    // swap elements 1 & 2, due to the structure of a COMPOSABLE_RAW_RUN
    uint16_t tmp0 = *q++;
    uint16_t tmp1 = *q--;
    *q++ = tmp1;
    *q = tmp0;

    // end of line with alignment padding
    *p++ = COMPOSABLE_EOL_ALIGN;
    *p++ = 0;

    buffers[j]->data_used = ((uint32_t *) p) - buffers[j]->data;
    // assert(buffer->data_used < buffer->data_max);

    buffers[j]->status = SCANLINE_OK;
    }
}



void core1_func() {

    // initialize video and interrupts on core 1
    scanvideo_setup(&vga_mode);
    scanvideo_timing_enable(true);
    sem_release(&video_initted);

    while (true) {
        #if 0
        const int num_scanlines = 4;
        scanvideo_scanline_buffer_t *scanline_buffers[num_scanlines];

        // fetch up to 8 scanlines
        for (int j=0; j<num_scanlines; j++) {
            scanvideo_scanline_buffer_t *scanline_buffer = scanvideo_begin_scanline_generation(true);
            scanline_buffers[j] = scanline_buffer;
        }


        // for (int j=0; j<num_scanlines; j++) {
            draw_scanline(scanline_buffers);
        // }

        for (int j=0; j<num_scanlines; j++) {
            scanvideo_end_scanline_generation(scanline_buffers[j]);
        }
        #else
            scanvideo_scanline_buffer_t *scanline_buffer = scanvideo_begin_scanline_generation(true);
            draw_scanline_fragment(scanline_buffer);
            scanvideo_end_scanline_generation(scanline_buffer);

        #endif
    }
}

int main(void) {
    set_sys_clock_khz(50000, true);

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

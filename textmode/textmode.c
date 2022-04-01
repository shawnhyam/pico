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

void core1_func();

void fastdraw_init();
inline void fastdrawchar(uint8_t c, uint line, uint16_t fgcolor, uint16_t bgcolor, void *buffer);
void drawchar(uint8_t c, uint line, uint16_t fgcolor, uint16_t bgcolor, void *buffer);

// Simple color bar program, which draws 7 colored bars: red, green, yellow, blow, magenta, cyan, white
// Can be used to check resister DAC correctness.
//
// Note this program also demonstrates running video on core 1, leaving core 0 free. It supports
// user input over USB or UART stdin, although all it does with it is invert the colors when you press SPACE

static semaphore_t video_initted;
static bool invert;
uint8_t screen[30][80];
uint16_t palette[16] = {
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

int main(void) {

    set_sys_clock_khz(250000, true);

    fastdraw_init();

    // create a semaphore to be posted when video init is complete
    sem_init(&video_initted, 0, 1);

    // launch all the video on core 1, so it isn't affected by USB handling on core 0
    multicore_launch_core1(core1_func);

    // wait for initialization of video to be complete
    sem_acquire_blocking(&video_initted);


    char *line = "Hello world";
    uint8_t *s = screen[0];
    for (int i=0; i<11; i++) {
        *s++ = line[i];
    }


    s = screen[1];
        char *line2 = "Goodbye";

    for (int i=0; i<7; i++) {
        *s++ = line2[i];
    }

    uint8_t c = 0;
    for (int y=0; y<30; y++) {
        for (int x=0; x<80; x++) {
            screen[y][x] = c++;
        }
    }

    while (true) {
        // prevent tearing when we invert - if you're astute you'll notice this actually causes
        // a fixed tear a number of scanlines from the top. this is caused by pre-buffering of scanlines
        // and is too detailed a topic to fix here.
        scanvideo_wait_for_vblank();
        int c = getchar_timeout_us(0);
        switch (c) {
            case ' ':
                invert = !invert;
                printf("Inverted: %d\n", invert);
                break;
        }
    }
}

uint8_t *font = AVGA2_8x16;
int palette_idx = 0;
int bg_idx = 32;

void draw_scanline(scanvideo_scanline_buffer_t *buffer) {
    // figure out 1/32 of the color value
    uint line_num = scanvideo_scanline_number(buffer->scanline_id);
    uint frame_num = scanvideo_frame_number(buffer->scanline_id);
   if (line_num % 16 == 0) {
       bg_idx++;
    }
    if (line_num == 0) {
        bg_idx = 0;
    }
       if (bg_idx >= 16) {
           bg_idx=0;
       }

    const int num_chars = 80;
    uint16_t bg = palette[bg_idx];

    uint16_t *p = (uint16_t *) buffer->data;
    uint16_t *q = p+1;
    *p++ = COMPOSABLE_RAW_RUN;
    *p++ = num_chars*8 - 3;
    for (int i=0; i<num_chars; i++) {
        uint16_t fg = palette[palette_idx++];
        if (palette_idx >= 16) { palette_idx=0; }
        fastdrawchar(screen[line_num/16][i], line_num % 16, fg, bg, (void *)p);
        p += 8;
    }

    // swap elements 1 & 2, due to the structure of a COMPOSABLE_RAW_RUN
    uint16_t tmp0 = *q++;
    uint16_t tmp1 = *q--;
    *q++ = tmp1;
    *q = tmp0;

    // black pixel to end line
    *p++ = COMPOSABLE_RAW_1P;
    *p++ = 0;
    // end of line with alignment padding
    *p++ = COMPOSABLE_EOL_SKIP_ALIGN;
    *p++ = 0;

    buffer->data_used = ((uint32_t *) p) - buffer->data;
    assert(buffer->data_used < buffer->data_max);

    buffer->status = SCANLINE_OK;

 
}

void core1_func() {
    stdio_init_all();

    // initialize video and interrupts on core 1
    scanvideo_setup(&vga_mode);
    scanvideo_timing_enable(true);
    sem_release(&video_initted);

    while (true) {
        scanvideo_scanline_buffer_t *scanline_buffer = scanvideo_begin_scanline_generation(true);
        draw_scanline(scanline_buffer);
        scanvideo_end_scanline_generation(scanline_buffer);
    }
}

uint32_t mask_table[256][4];

void fastdraw_init() {
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
}

// https://wiki.osdev.org/index.php?title=VGA_Fonts
inline void fastdrawchar(uint8_t c, uint line, uint16_t fgcolor, uint16_t bgcolor, void *buffer) {
	// uint8_t mask[8] = { 1,2,4,8,16,32,64,128 };
	uint8_t const glyph = font[c*16 + line];  // 16 bytes per glyph
 
	uint32_t bgcolor32 = bgcolor | (bgcolor << 16);
	uint32_t fgcolor32 = fgcolor | (fgcolor << 16);

    uint32_t *mask = mask_table[glyph];
    uint32_t *dest32 = buffer;

    uint32_t maskval = *mask++;
   *dest32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
   maskval = *mask++;
   *dest32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
   maskval = *mask++;
   *dest32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
   maskval = *mask++;
   *dest32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
}

inline void drawchar(uint8_t c, uint line, uint16_t fgcolor, uint16_t bgcolor, void *buffer) {
	uint8_t mask[8] = { 1,2,4,8,16,32,64,128 };
	uint8_t const glyph = font[c*16 + line];  // 16 bytes per glyph
 
    uint16_t *buf = buffer;

    // | RAW_RUN | COLOR1 | N-3 | COLOR2 | COLOR3 …​ | COLOR(N) |
    // *buf++ = COMPOSABLE_RAW_RUN;
    *buf++ = glyph & mask[7] ? fgcolor : bgcolor;
    // *buf++ = 5;
    *buf++ = glyph & mask[6] ? fgcolor : bgcolor;
    *buf++ = glyph & mask[5] ? fgcolor : bgcolor;
    *buf++ = glyph & mask[4] ? fgcolor : bgcolor;
    *buf++ = glyph & mask[3] ? fgcolor : bgcolor;
    *buf++ = glyph & mask[2] ? fgcolor : bgcolor;
    *buf++ = glyph & mask[1] ? fgcolor : bgcolor;
    *buf++ = glyph & mask[0] ? fgcolor : bgcolor;
}

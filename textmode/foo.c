#include "foo.h"
#include "AVGA2_8x16.h"

#include <stdio.h>
#include <stdint.h>

#include "pico.h"
#include "pico/stdlib.h"

static uint8_t *font; // = AVGA2_8x16;
static screen_char_t screen[30][80];
static color_t colors[30][80];

static uint32_t mask_table[256][4];

static uint32_t palette32[16];
static mixedcolor_t bigpalette[256];
static uint8_t pattern_buffer[480][80];


const int stride = 640;
const int stride_words = 320;


// fill 4 lines at a time with a single character
static inline void draw_character32(screen_char_t screen_char, int line, uint16_t *buffer) {
    uint8_t glyph = screen_char.glyph;
    uint32_t fgcolor32 = palette32[screen_char.color.v.fg];
    uint32_t bgcolor32 = palette32[screen_char.color.v.bg];

    pattern32_t *font_base = (pattern32_t *)(font + (glyph << 4) + line);
    pattern32_t pattern32 = *font_base;

    uint32_t *mask = mask_table[pattern32.p0];
    uint32_t *dst32 = (uint32_t *)(buffer);

    uint32_t maskval = *mask++;
    *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
    maskval = *mask++;
    *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
    maskval = *mask++;
    *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
    maskval = *mask++;
    *dst32 = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);

    mask = mask_table[pattern32.p1];
    dst32 += stride_words;
    
    maskval = *mask++;
    *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
    maskval = *mask++;
    *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
    maskval = *mask++;
    *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
    maskval = *mask++;
    *dst32 = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);

    mask = mask_table[pattern32.p2];
    dst32 += stride_words;

    maskval = *mask++;
    *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
    maskval = *mask++;
    *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
    maskval = *mask++;
    *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
    maskval = *mask++;
    *dst32 = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);

    mask = mask_table[pattern32.p3];
    dst32 += stride_words;

    maskval = *mask++;
    *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
    maskval = *mask++;
    *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
    maskval = *mask++;
    *dst32++ = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
    maskval = *mask++;
    *dst32 = (bgcolor32 & ~maskval) | (fgcolor32 & maskval);
}


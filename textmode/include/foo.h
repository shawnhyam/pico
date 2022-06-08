#include <stdio.h>
#include <stdint.h>

#include "pico.h"
#include "pico/stdlib.h"

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


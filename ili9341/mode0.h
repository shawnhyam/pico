
#ifndef _TEXT_MODE_H
#define _TEXT_MODE_H

// ARNE-16 palette -- https://lospec.com/palette-list/arne-16
typedef enum {
    MODE0_BLACK,
    MODE0_WOODBURN,
    MODE0_BRICK_RED,
    MODE0_DEEP_BLUSH,
    MODE0_DARK_GRAY
    // etc.
} mode0_color_t;


void mode0_init();
void mode0_clear();
void mode0_draw_screen();
void mode0_draw_region(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
void mode0_scroll_vertical(int8_t amount);
void mode0_scroll_horizontal(int8_t amount);
void mode0_set_foreground(mode0_color_t color);
void mode0_set_background(mode0_color_t color);
void mode0_set_cursor(uint8_t x, uint8_t y);
void mode0_print(char *s);
void mode0_putc(char c);

// maybe?
void mode0_begin();
void mode0_end();

#endif


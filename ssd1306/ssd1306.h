#ifndef _SSD1306_H_
#define _SSD1306_H_

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"


#define VIEW_HEIGHT 64
#define VIEW_WIDTH 256
#define VIEW_PAGES (VIEW_HEIGHT/8)

// interface to set bits directly on the screen buffer
uint ssd1306_init(i2c_inst_t *i2c, uint sda, uint scl, uint baud_rate);
void ssd1306_clear(uint8_t *buffer);
void ssd1306_display(uint8_t *buffer);
uint8_t* ssd1306_get_buffer();

// this creates a 256x64 view we can draw on
typedef struct {
    uint8_t cursor_x, cursor_y;
    uint8_t scroll_x, scroll_y;
    uint8_t map[VIEW_PAGES*VIEW_WIDTH];
} View;

void view_init(View *view);
void view_clear(View *view);
// color: 0 -> off, 1 -> on, -1 -> inverse
void view_draw_pixel(View *view, int16_t x, int16_t y, int8_t color);
void view_draw_letter_at(View *view, uint8_t x, uint8_t y, char c);
void view_print(View *view, const char* str);
void view_render(View *view);

#endif

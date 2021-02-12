#ifndef _SSD1306_H_
#define _SSD1306_H_

#include <stdint.h>
#include "pico/stdlib.h"


#define HEIGHT 32
#define WIDTH 128
#define PAGES (HEIGHT/8)

#define VIEW_HEIGHT 64
#define VIEW_WIDTH 256
#define VIEW_PAGES (VIEW_HEIGHT/8)


typedef struct {
    uint8_t cursor_x, cursor_y;
    uint8_t scroll_y;
    uint8_t render[PAGES*WIDTH+1];
    uint8_t map[VIEW_PAGES*VIEW_WIDTH];
} View;

uint init_i2c();
void init_display();

void view_init(View *view);
void view_render(View *view);
void view_print(View *view, const char* str);


#endif

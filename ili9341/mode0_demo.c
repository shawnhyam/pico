#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "mode0.h"
#include "ili9341.h"


int main() {
    mode0_init();
    
    mode0_set_cursor(0, 0);
    mode0_color_t fg = MODE0_WHITE;
    mode0_color_t bg = MODE0_BLACK;
    
    while (1) {
        mode0_print("My Computer (c) 2021, Shawn Hyam\n");
        sleep_ms(200);
        fg = (fg+1) % 16;
        if (fg == 0) {
            bg = (bg+1) % 16;
            mode0_set_background(bg);
            mode0_clear(bg);
        }
        mode0_set_foreground(fg);

    }
}

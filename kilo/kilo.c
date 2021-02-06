#include "pico/stdlib.h"
#include "pico/printf.h"

#include "../ili9341/mode0.h"
#include "pico/stdio.h"
#include <ctype.h>


/* DEFINES */

#define CTRL_KEY(k) ((k) & 0x1f)

/* */

// get character from input
// show ascii code for character on the display
// repeat

void kilo_print(const char *s) {
    mode0_print(s);
    printf("%s", s);
}

void kilo_die(const char *s) {
    mode0_begin();
    mode0_clear(MODE0_RED);
    mode0_set_background(MODE0_RED);
    mode0_set_foreground(MODE0_WHITE);
    mode0_print(s);
    mode0_end();
    
    while(1) {
        sleep_ms(1000);
    }
}

char kilo_read_key() {
    int ch = getchar_timeout_us(100 * 1000);
    if (ch == PICO_ERROR_TIMEOUT) { return 0; }
    return ch;
}

void kilo_process_keypress() {
    char c = kilo_read_key();
    switch (c) {
        case CTRL_KEY('q'):
            kilo_die("User terminated program.");
            break;
    }
    
    char buffer[256];
    if (iscntrl(c)) {
        sprintf(buffer, "%d\n", c);
    } else {
        sprintf(buffer, "%d ('%c')\n", c, c);
    }
    kilo_print(buffer);
}

int main() {
    mode0_init();
    stdio_init_all();

    
    while (1) {
        kilo_process_keypress();
    }
    
    return 0;
}

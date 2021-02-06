#include "pico/stdlib.h"
#include "pico/printf.h"

#include "../ili9341/mode0.h"
#include "pico/stdio.h"
#include <ctype.h>


/* DEFINES */
#define KILO_VERSION "0.0.1"
#define CTRL_KEY(k) ((k) & 0x1f)

// FIXME get these from mode0
#define MODE0_HEIGHT 24
#define MODE0_WIDTH 53

typedef enum {
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN
} kilo_key_t;

/* DATA */

typedef struct  {
    int cx, cy;  // cursor position
    int screen_rows;
    int screen_cols;
    //struct termios orig_termios;
} editor_config_t;

static editor_config_t E;

/* INIT */

void kilo_init() {
    E.cx = 0;
    E.cy = 0;
    E.screen_cols = MODE0_WIDTH;
    E.screen_rows = MODE0_HEIGHT;
    
    mode0_show_cursor();
}

/* */
// get character from input
// show ascii code for character on the display
// repeat

//void kilo_print(const char *s) {
//    mode0_print(s);
//    printf("%s", s);
//}

void kilo_write(const char *s, int len) {
    mode0_write(s, len);
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

/* INPUT */

int kilo_read_key() {
    int ch = getchar_timeout_us(100 * 1000);
    if (ch == PICO_ERROR_TIMEOUT) { return 0; }
    
    switch (ch) {
        case 28: return ARROW_LEFT;
        case 29: return ARROW_RIGHT;
        case 30: return ARROW_UP;
        case 31: return ARROW_DOWN;
    }
    
    return ch;
}

void kilo_move_cursor(int key) {
    switch (key) {
        case ARROW_LEFT:
            E.cx--;
            break;
        case ARROW_RIGHT:
            E.cx++;
            break;
        case ARROW_UP:
            E.cy--;
            break;
        case ARROW_DOWN:
            E.cy++;
            break;
    }
}

void kilo_process_keypress() {
    int c = kilo_read_key();
    switch (c) {
        case CTRL_KEY('q'):
            kilo_die("User terminated program.");
            break;
            
        case ARROW_LEFT:
        case ARROW_RIGHT:
        case ARROW_UP:
        case ARROW_DOWN:
          kilo_move_cursor(c);
          break;
    }
    
    
    if (iscntrl(c)) {
        printf("%d\n", c);
    } else {
        printf("%d ('%c')\n", c, c);
    }
}

void kilo_draw_rows() {
    mode0_begin();
    for (int y = 0; y < E.screen_rows; y++) {
        mode0_set_cursor(0, y);
        
        if (y == E.screen_rows / 3) {
            char welcome[80];
            int welcome_len = snprintf(welcome, sizeof(welcome),
                                      "Kilo editor -- version %s", KILO_VERSION);

            if (welcome_len > E.screen_cols)
                welcome_len = E.screen_cols;
            welcome[79] = 0;
            
            int padding = (E.screen_cols - welcome_len) / 2;
            if (padding) {
                kilo_write("~", 1);
                padding--;
            }
            while (padding--)
                kilo_write(" ", 1);
            
            kilo_write(welcome, welcome_len);
        } else {
            kilo_write("~", 1);
        }
    }
    mode0_end();
}

void kilo_refresh_screen() {
    mode0_begin();
    mode0_clear(MODE0_BLACK);
    kilo_draw_rows();
    mode0_set_cursor(E.cx, E.cy);
    mode0_end();
}

int main() {
    mode0_init();
    stdio_init_all();
    kilo_init();

    
    while (1) {
        kilo_refresh_screen();
        kilo_process_keypress();
    }
    
    return 0;
}

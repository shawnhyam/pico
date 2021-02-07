#include "pico/stdlib.h"
#include "pico/printf.h"

#include "../ili9341/mode0.h"
#include "pico/stdio.h"
#include <ctype.h>
#include <stdlib.h>  // ??
#include <string.h>  // ??



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
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN
} kilo_key_t;

/* DATA */

typedef struct {
    int size;
    char *chars;
} erow_t;

typedef struct  {
    int cx, cy;  // cursor position
    int row_offset;
    int col_offset;
    int screen_rows;
    int screen_cols;
    int num_rows;
    erow_t *row;
    //struct termios orig_termios;
} editor_config_t;

static editor_config_t E;

/* ROW OPERATIONS*/

void kilo_append_row(char *s, size_t len) {
    E.row = realloc(E.row, sizeof(erow_t) * (E.num_rows + 1));
    int at = E.num_rows;
    E.row[at].size = len;
    E.row[at].chars = malloc(len + 1);
    memcpy(E.row[at].chars, s, len);
    E.row[at].chars[len] = '\0';
    E.num_rows++;
}

/* FILE?? I/O */

void kilo_open() {
    char *line = "Hello, world!";
    kilo_append_row(line, 13);
    char buffer[100];
    for (int i=0; i<50; i++) {
        sprintf(buffer, "This is another row of text: %d", i);
        kilo_append_row(buffer, strlen(buffer));
    }
}

/* INIT */

void kilo_init() {
    E.cx = 0;
    E.cy = 0;
    E.row_offset = 0;
    E.col_offset = 0;
    E.num_rows = 0;
    E.screen_cols = MODE0_WIDTH;
    E.screen_rows = MODE0_HEIGHT;
    E.row = NULL;
    
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
            if (E.cx != 0) {
                E.cx--;
            }
            break;
        case ARROW_RIGHT:
//            if (E.cx != E.screen_cols - 1) {
                E.cx++;
//            }
            break;
        case ARROW_UP:
            if (E.cy != 0) {
                E.cy--;
            }
            break;
        case ARROW_DOWN:
            if (E.cy < E.num_rows) {
                E.cy++;
            }
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
            
        case HOME_KEY:
            E.cx = 0;
            break;
        case END_KEY:
            E.cx = E.screen_cols - 1;
            break;

        case PAGE_UP:
        case PAGE_DOWN:
        {
            int times = E.screen_rows;
            while (times--)
                kilo_move_cursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
        }
            break;
            
    }
    
    
    if (iscntrl(c)) {
        printf("%d\n", c);
    } else {
        printf("%d ('%c')\n", c, c);
    }
}

/* OUTPUT */

void kilo_scroll() {
    if (E.cy < E.row_offset) {
        E.row_offset = E.cy;
    }
    if (E.cy >= E.row_offset + E.screen_rows) {
        E.row_offset = E.cy - E.screen_rows + 1;
    }
    if (E.cx < E.col_offset) {
        E.col_offset = E.cx;
    }
    if (E.cx >= E.col_offset + E.screen_cols) {
        E.col_offset = E.cx - E.screen_cols + 1;
    }
}

void kilo_draw_rows() {
    mode0_begin();
    for (int y = 0; y < E.screen_rows; y++) {
        mode0_set_cursor(0, y);

        int file_row = y + E.row_offset;
        if (file_row >= E.num_rows) {
            if (E.num_rows == 0 && y == E.screen_rows / 3) {
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
        } else {
            int len = E.row[file_row].size - E.col_offset;
            if (len < 0) len = 0;
            if (len > E.screen_cols) len = E.screen_cols;
            kilo_write(&E.row[file_row].chars[E.col_offset], len);

        }
    }
    mode0_end();
}

void kilo_refresh_screen() {
    mode0_begin();
    mode0_clear(MODE0_BLACK);
    
    kilo_scroll();
    kilo_draw_rows();
    
    mode0_set_cursor(E.cx - E.col_offset, E.cy - E.row_offset);
    mode0_end();
}

int main() {
    mode0_init();
    stdio_init_all();
    kilo_init();
    kilo_open();

    
    while (1) {
        kilo_refresh_screen();
        kilo_process_keypress();
    }
    
    return 0;
}

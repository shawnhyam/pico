#include "kilo_platform.h"
#include "kilo.h"
#include "mode0/mode0.h"

#include "pico/stdlib.h"
#include "pico/printf.h"
#include "pico/stdio.h"

#include <ctype.h>
#include <stdlib.h>  // ??
#include <string.h>  // ??


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

void kilo_write(const char *s, int len) {
    mode0_write(s, len);
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
            int len = E.row[file_row].rsize - E.col_offset;
            if (len < 0) len = 0;
            if (len > E.screen_cols) len = E.screen_cols;
            kilo_write(&E.row[file_row].render[E.col_offset], len);
        }
    }
    mode0_end();
}

void kilo_draw_status_bar() {
    mode0_set_background(MODE0_WHITE);
    mode0_set_foreground(MODE0_BLACK);
    mode0_set_cursor(0, E.screen_rows-1);

    char status[80], rstatus[80];
     int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
        E.filename ? E.filename : "[No Name]", E.num_rows,
        E.dirty ? "(modified)" : "");

    int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d",
                        E.cy + 1, E.num_rows);
    if (len > E.screen_cols) len = E.screen_cols;
    mode0_write(status, len);
    while (len < E.screen_cols) {
        if (E.screen_cols - len == rlen) {
            mode0_write(rstatus, rlen);
            break;
        } else {
            mode0_write(" ", 1);
            len++;
        }
    }

    mode0_set_background(MODE0_BLACK);
    mode0_set_foreground(MODE0_WHITE);
}

void kilo_draw_message_bar() {
    mode0_set_cursor(0, E.screen_rows);

    int msglen = strlen(E.statusmsg);
    if (msglen > E.screen_cols) msglen = E.screen_cols;
    if (msglen && get_absolute_time() - E.statusmsg_time < 5000000)
        mode0_write(E.statusmsg, msglen);
}

void kilo_refresh_screen() {
    mode0_begin();
    mode0_clear(MODE0_BLACK);

    kilo_scroll();
    kilo_draw_rows();
    kilo_draw_status_bar();
    kilo_draw_message_bar();

    mode0_set_cursor(E.rx - E.col_offset, E.cy - E.row_offset);
    mode0_end();
}

void kilo_display_init() {
    mode0_init();
    mode0_show_cursor();
}

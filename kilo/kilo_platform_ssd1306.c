#include "kilo_platform.h"
#include "kilo.h"
#include "ssd1306/ssd1306.h"

#include "pico/printf.h"

static View view;

void kilo_die(const char* s) {
}

void kilo_write(const char *s, int len) {
	view_write(&view, s, len);
}

void kilo_draw_rows() {
    for (int y=0; y<4; y++) {
		view.cursor_x = 0;
		view.cursor_y = y*8;

		int file_row = y + E.row_offset;
		if (file_row >= E.num_rows) {
    		if (E.num_rows == 0 && y == 1) {
        		char welcome[32];
        		int welcome_len = snprintf(welcome, sizeof(welcome),
        		                           "Kilo version %s", KILO_VERSION);
        		if (welcome_len > E.screen_cols)
            		welcome_len = E.screen_cols;
        		welcome[31] = 0;

        		int padding = (E.screen_cols - welcome_len)/2;
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
    		if (len<0) len=0;
    		if (len>E.screen_cols) len = E.screen_cols;
    		kilo_write(&E.row[file_row].render[E.col_offset], len);
		}
    }
}

void kilo_draw_status_bar() {
}

void kilo_draw_message_bar() {
}

void kilo_refresh_screen() {
    view_clear(&view);

	kilo_scroll();
	kilo_draw_rows();
	kilo_draw_status_bar();
	kilo_draw_message_bar();

    view.cursor_x = (E.rx - E.col_offset)*6;
    view.cursor_y = (E.cy - E.row_offset)*8;
	view_render(&view);
}

void kilo_display_init() {
	ssd1306_init(i2c0, 12, 13, 1984*1000);
	view_init(&view);
	E.screen_cols = 21;
	E.screen_rows = 3;

}


#include "pico/stdlib.h"
#include "pico/printf.h"

#include "../ili9341/mode0.h"
#include "pico/stdio.h"
#include <ctype.h>
#include <stdlib.h>  // ??
#include <string.h>  // ??
#include "hardware/flash.h"

// We're going to erase and reprogram a region 256k from the start of flash.
// Once done, we can access this at XIP_BASE + 256k.
#define FLASH_TARGET_OFFSET (770 * 1024)
const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);

/* DEFINES */
#define KILO_VERSION "0.0.1"
#define KILO_TAB_STOP 4
#define KILO_QUIT_TIMES 3
#define CTRL_KEY(k) ((k) & 0x1f)

// FIXME get these from mode0
#define MODE0_HEIGHT 24
#define MODE0_WIDTH 53

typedef enum {
    BACKSPACE = 127,
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
    int rsize;  // rendered size
    char *chars;
    char *render;
} erow_t;

typedef struct  {
    int cx, cy;  // cursor position
    int rx; // cursor position on the render string
    int row_offset;
    int col_offset;
    int screen_rows;
    int screen_cols;
    int num_rows;
    int dirty;
    erow_t *row;
    char *filename;
    char statusmsg[80];
    absolute_time_t statusmsg_time;
} editor_config_t;

static editor_config_t E;


/* PROTOTYPES */

void kilo_set_status_message(const char *fmt, ...);
void kilo_init();
void kilo_refresh_screen();

/* ROW OPERATIONS*/

int kilo_row_cx_to_rx(erow_t *row, int cx) {
    int rx = 0;
    for (int j = 0; j < cx; j++) {
        if (row->chars[j] == '\t')
            rx += (KILO_TAB_STOP - 1) - (rx % KILO_TAB_STOP);
        rx++;
    }
    return rx;
}

void kilo_update_row(erow_t *row) {
    int tabs = 0;
    for (int j = 0; j < row->size; j++) {
        if (row->chars[j] == '\t') tabs++;
    }

    free(row->render);
    row->render = malloc(row->size+ tabs*(KILO_TAB_STOP-1) + 1);
    int idx = 0;
    for (int j=0; j<row->size; j++) {
        if (row->chars[j] == '\t') {
            row->render[idx++] = ' ';
            while (idx % KILO_TAB_STOP != 0) row->render[idx++] = ' ';
        } else {
            row->render[idx++] = row->chars[j];
        }
    }
    row->render[idx] = '\0';
    row->rsize = idx;
}

void kilo_insert_row(int at, const char *s, size_t len) {
    if (at<0 || at>E.num_rows) return;

    E.row = realloc(E.row, sizeof(erow_t) * (E.num_rows + 1));
    memmove(&E.row[at+1], &E.row[at], sizeof(erow_t) * (E.num_rows - at));

    E.row[at].size = len;
    E.row[at].chars = malloc(len + 1);
    memcpy(E.row[at].chars, s, len);
    E.row[at].chars[len] = '\0';

    E.row[at].rsize = 0;
    E.row[at].render = NULL;
    kilo_update_row(&E.row[at]);

	E.dirty++;
    E.num_rows++;
}

void kilo_free_row(erow_t *row) {
    free(row->render);
    free(row->chars);
}

void kilo_del_row(int at) {
	if (at<0 || at>=E.num_rows) return;
	kilo_free_row(&E.row[at]);
	memmove(&E.row[at], &E.row[at+1], sizeof(erow_t) * (E.num_rows - at - 1));
	E.num_rows--;
	E.dirty++;
}

void kilo_row_insert_char(erow_t *row, int at, int c) {
    if (at < 0 || at > row->size) at = row->size;
    row->chars = realloc(row->chars, row->size + 2);
    memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
    row->size++;
    row->chars[at] = c;
    kilo_update_row(row);
    E.dirty++;
}

void kilo_row_append_string(erow_t *row, char *s, size_t len) {
    row->chars = realloc(row->chars, row->size + len + 1);
    memcpy(&row->chars[row->size], s, len);
    row->size += len;
    row->chars[row->size] = '\0';
    kilo_update_row(row);
    E.dirty++;
}

void kilo_row_del_char(erow_t *row, int at) {
	if (at < 0 || at > row->size) return;
	memmove(&row->chars[at], &row->chars[at+1], row->size-at);
	row->size--;
	kilo_update_row(row);
	E.dirty++;
}


/* EDITOR OPERATIONS */

void kilo_insert_char(int c) {
    if (E.cy == E.num_rows) {
        kilo_insert_row(E.num_rows, "", 0);
    }
    kilo_row_insert_char(&E.row[E.cy], E.cx, c);
    E.cx++;
}

void kilo_insert_newline() {
    if (E.cx == 0) {
        kilo_insert_row(E.cy, "", 0);
    } else {
        erow_t *row = &E.row[E.cy];
        kilo_insert_row(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
        row = &E.row[E.cy];
        row->size = E.cx;
        row->chars[row->size] = '\0';
        kilo_update_row(row);
    }
    E.cy++;
    E.cx = 0;
}

void kilo_del_char() {
    if (E.cy == E.num_rows) return;
	if (E.cx == 0 && E.cy == 0) return;

    erow_t *row = &E.row[E.cy];
    if (E.cx > 0) {
        kilo_row_del_char(row, E.cx-1);
        E.cx--;
    } else {
        E.cx = E.row[E.cy-1].size;
        kilo_row_append_string(&E.row[E.cy-1], row->chars, row->size);
        kilo_del_row(E.cy);
        E.cy--;
    }
}


/* FILE?? I/O */

void kilo_open() {
	kilo_init();

	free(E.filename);
    E.filename = strdup("filename.txt");

	char buf[21] = { 0 };
	memcpy(buf, flash_target_contents, 20);

	const char *line = flash_target_contents;
	int idx = 0;
	//const char foo[] = "Hello World.\nLet's see if this will load properly.\n";
	//const cha[Br *line_start = foo;
	while (line[idx] != '\0') {
		if (line[idx] == '\r' || line[idx] == '\n') {
	    	kilo_insert_row(E.num_rows, line, idx);
    		line += idx;
    		line += 1;
    		idx = 0;
		} else {
    		idx++;
		}
	}

	E.dirty = 0;
}


char *kilo_rows_to_string(int *buflen) {
    int totlen = 0;
    for (int j = 0; j < E.num_rows; j++) {
        totlen += E.row[j].size + 1;
    }
    *buflen = totlen;
    char *buf = malloc(totlen);
    char *p = buf;
    for (int j = 0; j < E.num_rows; j++) {
        memcpy(p, E.row[j].chars, E.row[j].size);
        p += E.row[j].size;
        *p = '\n';
        p++;
    }
    return buf;
}

void kilo_save() {
    //if (E.filename == NULL) {
	//	return;
	//}

	char tmp[FLASH_PAGE_SIZE] = { 0 };
	int len;
	char *buf = kilo_rows_to_string(&len);
	memcpy(tmp, buf, len);

	flash_range_program(FLASH_TARGET_OFFSET, tmp, FLASH_PAGE_SIZE);

	int failed = 0;
	for (int i=0; i<len; i++) {
    	if (buf[i] != flash_target_contents[i]) {
        	failed++;
    	}
	}

	kilo_set_status_message("%d %d", failed, len);

	E.dirty = 0;
	free(buf);

}


/* INIT */

void kilo_init() {
    E.cx = 0;
    E.cy = 0;
    E.rx = 0;
    E.row_offset = 0;
    E.col_offset = 0;
    E.num_rows = 0;
    E.screen_cols = MODE0_WIDTH;
    E.screen_rows = MODE0_HEIGHT-2;
    E.dirty = 0;
    E.row = NULL;
    E.filename = NULL;
    E.statusmsg[0] = '\0';
    E.statusmsg_time = 0;

    mode0_show_cursor();
}

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
    erow_t *row = (E.cy >= E.num_rows) ? NULL : &E.row[E.cy];

    switch (key) {
        case ARROW_LEFT:
            if (E.cx != 0) {
                E.cx--;
            } else if (E.cy > 0) {
                // move to the far right of the previous line
                E.cy--;
                E.cx = E.row[E.cy].size;
            }
            break;
        case ARROW_RIGHT:
            if (row && E.cx < row->size) {
                E.cx++;
            } else if (row && E.cx == row->size) {
                // move to the far left of the following line
                E.cy++;
                E.cx = 0;
            }
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

    // snap cursor to the end of the line when changing lines
    row = (E.cy >= E.num_rows) ? NULL : &E.row[E.cy];
    int row_len = row ? row->size : 0;
    if (E.cx > row_len) {
        E.cx = row_len;
    }
}

void kilo_process_keypress() {
	static int quit_times = KILO_QUIT_TIMES;

	int c = kilo_read_key();
    switch (c) {
        case 0:
            return;
        case '\r':
            kilo_insert_newline();
            break;

        case CTRL_KEY('q'):
        	if (E.dirty && quit_times > 0) {
				kilo_set_status_message("WARNING! File has unsaved changes. "
					"Press Ctrl-Q %d more times to quit.", quit_times);
				quit_times--;
				return;
        	}
            kilo_die("User terminated program.");
            break;

        case CTRL_KEY('s'):
        	kilo_save();
        	break;

        case CTRL_KEY('o'):
        	kilo_open();
        	break;

        case BACKSPACE:
        case CTRL_KEY('h'):
        case DEL_KEY:
			if (c == DEL_KEY) kilo_move_cursor(ARROW_RIGHT);
			kilo_del_char();
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
        case CTRL_KEY('l'):
        case '\x1b':
        	break;

        default:
            kilo_insert_char(c);
            break;

    }

	quit_times = KILO_QUIT_TIMES;

    if (iscntrl(c)) {
        printf("%d\n", c);
    } else {
        printf("%d ('%c')\n", c, c);
    }
}

/* OUTPUT */

void kilo_scroll() {
    E.rx = 0;
    if (E.cy < E.num_rows) {
        E.rx = kilo_row_cx_to_rx(&E.row[E.cy], E.cx);
    }

    if (E.cy < E.row_offset) {
        E.row_offset = E.cy;
    }
    if (E.cy >= E.row_offset + E.screen_rows) {
        E.row_offset = E.cy - E.screen_rows + 1;
    }
    if (E.rx < E.col_offset) {
        E.col_offset = E.rx;
    }
    if (E.rx >= E.col_offset + E.screen_cols) {
        E.col_offset = E.rx - E.screen_cols + 1;
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

void kilo_set_status_message(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
    va_end(ap);
    E.statusmsg_time = get_absolute_time();
}

int main() {
    mode0_init();
    stdio_init_all();
    kilo_init();
    // kilo_open();

    kilo_set_status_message("HELP: Ctrl-S = save | Ctrl-Q = quit");

    while (1) {
        kilo_refresh_screen();
        kilo_process_keypress();
    }

    return 0;
}

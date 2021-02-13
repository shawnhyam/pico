#include <stdint.h>
#include <string.h>
#include "ssd1306.h"

extern const uint8_t ssd1306_font6x8[];

#define SET_CONTRAST 0x81
#define SET_ENTIRE_ON 0xA4
#define SET_NORM_INV 0xA6
#define SET_DISP 0xAE
#define SET_MEM_ADDR 0x20
#define SET_COL_ADDR 0x21
#define SET_PAGE_ADDR 0x22
#define SET_DISP_START_LINE 0x40
#define SET_SEG_REMAP 0xA0
#define SET_MUX_RATIO 0xA8
#define SET_COM_OUT_DIR 0xC0
#define SET_DISP_OFFSET 0xD3
#define SET_COM_PIN_CFG 0xDA
#define SET_DISP_CLK_DIV 0xD5
#define SET_PRECHARGE 0xD9
#define SET_VCOM_DESEL 0xDB
#define SET_CHARGE_PUMP 0x8D


#define HEIGHT 32
#define WIDTH 128
#define PAGES (HEIGHT/8)

#define SID 0x3C // I2C 1306 slave ID

static uint8_t buffer[PAGES*WIDTH+1] = { 0x40, 0 };
static i2c_inst_t *i2c_inst;

void send2(uint8_t v1, uint8_t v2) {
    uint8_t buf[2];
    buf[0] = v1;
    buf[1] = v2;
    i2c_write_blocking(i2c_inst, SID, buf, 2, false);
}

void write_cmd(uint8_t cmd) {
    send2(0x80, cmd);
}


void ssd1306_display(uint8_t *buffer) {
    //write_cmd(SET_MEM_ADDR); // 0x20
    write_cmd(0b01); // vertical addressing mode

    write_cmd(SET_COL_ADDR); // 0x21
    write_cmd(0);
    write_cmd(127);

    write_cmd(SET_PAGE_ADDR); // 0x22
    write_cmd(0);
    write_cmd(7);

    i2c_write_blocking(i2c_inst, SID, buffer, PAGES*WIDTH+1, false);
}

// for example: i2c0, 16, 17, 1024*1024
uint ssd1306_init(i2c_inst_t *i2c, uint sda, uint scl, uint baud_rate) {
	i2c_inst = i2c;

	// set up the i2c connection
	uint actual_baud_rate = i2c_init(i2c, baud_rate);
	gpio_set_function(sda, GPIO_FUNC_I2C);
	gpio_set_function(scl, GPIO_FUNC_I2C);
	gpio_pull_up(sda);
	gpio_pull_up(scl);

	// initialize the display
	const bool external_vcc = false;

	uint8_t cmds[] = {
		SET_DISP | 0x00,  // display off 0x0E | 0x00

		SET_MEM_ADDR, // 0x20
		0x00,  // horizontal

		//# resolution and layout
		SET_DISP_START_LINE | 0x00, // 0x40
		SET_SEG_REMAP | 0x01,  //# column addr 127 mapped to SEG0

		SET_MUX_RATIO, // 0xA8
		0x1f,

		SET_COM_OUT_DIR | 0x08,  //# scan from COM[N] to COM0  (0xC0 | val)
		SET_DISP_OFFSET, // 0xD3
		0x00,

		SET_COM_PIN_CFG, // 0xDA
        0x02,
		//0x02 if self.width > 2 * self.height else 0x12,
		//width > 2*height ? 0x02 : 0x12,
		//SET_COM_PIN_CFG, height == 32 ? 0x02 : 0x12,

		//# timing and driving scheme
		SET_DISP_CLK_DIV, // 0xD5
		0x80,

		SET_PRECHARGE,
		//0x22 if self.external_vcc else 0xF1,
		external_vcc ? 0x22 : 0xF1,

		SET_VCOM_DESEL,
		//0x30,  //# 0.83*Vcc
		0x40, // changed by mcarter

		//# display
		SET_CONTRAST, // 0x81
		0xFF,  //# maximum

		SET_ENTIRE_ON,  //# output follows RAM contents // 0xA4
		SET_NORM_INV,  //# not inverted 0xA6

		SET_CHARGE_PUMP, // 0x8D
		//0x10 if self.external_vcc else 0x14,
		external_vcc ? 0x10 : 0x14,

		SET_DISP | 0x01
	};

    i2c_write_blocking(i2c, SID, cmds, sizeof(cmds), false);
	return actual_baud_rate;
}

void view_init(View *view) {
    view->cursor_x = 0;
    view->cursor_y = 0;
    view->scroll_y = 0;
    memset(view->map, 0, VIEW_PAGES*VIEW_WIDTH);
}

void view_render(View *view) {
    // for now, we have to be page-aligned and horizontal scroll isn't implemented
    assert(view->scroll_y % 8 == 0);
    assert(view->scroll_x == 0);

    for (int page=0, map_page = view->scroll_y / 8; page<PAGES; page++, map_page++) {
        map_page = map_page % VIEW_PAGES;  // wrap around
        memcpy(buffer + page*WIDTH+1, view->map + map_page*VIEW_WIDTH, WIDTH);
    }

	ssd1306_display(buffer);
}

void view_clear(View *view) {
    memset(view->map, 0, VIEW_PAGES*VIEW_WIDTH);
}

void view_draw_pixel(View* view, int16_t x, int16_t y, int8_t color) {
    if (x<0 || x >= VIEW_WIDTH || y<0 || y>= VIEW_HEIGHT) return;

    int page = y/8;
    int bit = 1<<(y % 8);

    uint8_t* ptr = view->map + x + (page * VIEW_WIDTH);

    switch (color) {
        case 1: // white
            *ptr |= bit;
            break;
        case 0: // black
            *ptr &= ~bit;
            break;
        case -1: //inverse
            *ptr ^= bit;
            break;
    }
}

void view_draw_letter_at(View *view, uint8_t x, uint8_t y, char c) {
    if (c < ' ' || c > 0x7F) c = '?'; // 0x7F is the DEL key

    int offset = 4 + (c - ' ' )*6;
    for (int col=0; col<6; col++) {
        uint8_t line = ssd1306_font6x8[offset+col];
        // FIXME do this a whole byte at a time, not one pixel
        for (int row = 0; row <8; row++) {
            view_draw_pixel(view, x+col, y+row, line & 1);
            line >>= 1;
        }
    }
}

void view_print(View *view, const char* str) {
    char c;
    while (c = *str) {
        str++;
        if (c == '\n') {
            view->cursor_x = 0;
            view->cursor_y += 8;
            continue;
        }
        // TODO adjust scroll_y if necessary
        view_draw_letter_at(view, view->cursor_x, view->cursor_y, c);
        view->cursor_x += 6;
    }
}




static uint8_t scr[PAGES*WIDTH+1]; // extra byte holds data send instruction

void fill_scr(uint8_t* scr, uint8_t v) {
	memset(scr+1, v, PAGES*WIDTH);
}


void poweroff() { write_cmd(SET_DISP | 0x00); }

void poweron() { write_cmd(SET_DISP | 0x01); }

void contrast(uint8_t contrast) { write_cmd(SET_CONTRAST); write_cmd(contrast); }

void invert(uint8_t invert) { write_cmd(SET_NORM_INV | (invert & 1)); }

void draw_pixel(uint8_t* scr, int16_t x, int16_t y, int color)
{
	if(x<0 || x >= WIDTH || y<0 || y>= HEIGHT) return;

	int page = y/8;
	int bit = 1<<(y % 8);

    uint8_t* ptr = scr + x + (page * 128) + 1;
	//uint8_t* ptr = scr + x + page  + 1;

	switch (color) {
		case 1: // white
			*ptr |= bit;
			break;
		case 0: // black
			*ptr &= ~bit;
			break;
		case -1: //inverse
			*ptr ^= bit;
			break;
	}

}

uint8_t get_pixel(uint8_t* scr, int16_t x, int16_t y) {
    if(x<0 || x >= WIDTH || y<0 || y>= HEIGHT) return 0;

    int page = y/8;
    int bit = 1<<(y % 8);

    return (scr[x + page*128 + 1] & bit) ? 1 : 0;
}



void draw_letter_at(uint8_t x, uint8_t y, char c)
{
	//char c = 'C';
	//c= 'Q';
	if(c< ' ' || c>  0x7F) c = '?'; // 0x7F is the DEL key

	int offset = 4 + (c - ' ' )*6;
	for(int col = 0 ; col < 6; col++) {
		uint8_t line =  ssd1306_font6x8[offset+col];
		for(int row =0; row <8; row++) {
			//char x = (line & (1<<7)) ? 'X' : '_';
			draw_pixel(scr, x+col, y+row, line & 1);
			//cout << x;
			line >>= 1;
		}
		//cout << '\n';
	}
}

int cursorx = 0, cursory = 0;
void ssd1306_print(const char* str)
{
	char c;
	while(c = *str) {
		str++;
		if(c == '\n') {
			cursorx = 0;
			cursory += 8;
			continue;
		}
		draw_letter_at(cursorx, cursory, c);
		cursorx += 6;
	}
}

void ssd1306_scroll_y(int8_t offset) {

   // uint8_t newscr[PAGES*WIDTH+1]; // extra byte holds data send instruction
    //fill_scr(newscr, 0);


    for (uint8_t y=0; y<32-offset; y++) {
        for (uint8_t x=0; x<128; x++) {
            //draw_pixel(newscr, x, y, get_pixel(scr, x, y+offset));
            draw_pixel(scr, x, y, get_pixel(scr, x, y+offset));
        }
    }
    for (uint8_t y=32-offset; y<32; y++) {
        for (uint8_t x=0; x<128; x++) {
            draw_pixel(scr, x, y, 0);
        }
    }

    cursory -= offset;
    //fill_scr(scr, 85);

//    for (uint8_t i=1; i<PAGES*WIDTH+1; i++) {
//        scr[i] = newscr[i];
//    }

}

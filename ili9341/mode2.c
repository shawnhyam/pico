#include "mode2/mode2.h"
#include <string.h>

#define SIZE (ILI9341_TFTHEIGHT*ILI9341_TFTWIDTH)

uint16_t mode2_buffer[SIZE] = { 0 };

void mode2_init() {
}

void mode2_clear() {
    memset(mode2_buffer, 0, SIZE*sizeof(uint16_t));
}

void mode2_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
	uint16_t *base_loc = &mode2_buffer[x*ILI9341_TFTWIDTH+y];

	for (int h=0; h<width; h++) {
	    uint16_t *loc = base_loc + h*ILI9341_TFTWIDTH;
    	for (int v=0; v<height; v++) {
			*loc++ = color;
    	}
	}
}

void mode2_setpixel(uint16_t x, uint16_t y, uint16_t color) {
	
	// only if it's inside the bounds
	if(x>=0 && y>=0 && x< ILI9341_TFTWIDTH && y<ILI9341_TFTHEIGHT){
	   uint16_t *base_loc = &mode2_buffer[x*ILI9341_TFTWIDTH+y];
	   *base_loc = color;
	}
    	
}


void mode2_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
	

  int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1; 
  int err = dx + dy, e2; /* error value e_xy */
 
  for (;;)
  {  /* loop */
    mode2_setpixel(x0,y0,color);
    if (x0 == x1 && y0 == y1) break;
    e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
    if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
  }
}

	
void mode2_circle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t color){
	
  int f = 1 - radius;
  int ddF_x = 1;
  int ddF_y = -2 * radius;
  int x = 0;
  int y = radius;
  
 
  mode2_setpixel(x0, y0 + radius,color);
  mode2_setpixel(x0, y0 - radius,color);
  mode2_setpixel(x0 + radius, y0,color);
  mode2_setpixel(x0 - radius, y0,color);
  while (x < y)
  {
    // ddF_x == 2 * x + 1;
    // ddF_y == -2 * y;
    // f == x*x + y*y - radius*radius + 2*x - y + 1;
    if (f >= 0) 
    {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;    
    mode2_setpixel(x0 + x, y0 + y,color);
    mode2_setpixel(x0 - x, y0 + y,color);
    mode2_setpixel(x0 + x, y0 - y,color);
    mode2_setpixel(x0 - x, y0 - y,color);
    mode2_setpixel(x0 + y, y0 + x,color);
    mode2_setpixel(x0 - y, y0 + x,color);
    mode2_setpixel(x0 + y, y0 - x,color);
    mode2_setpixel(x0 - y, y0 - x,color);
  }
	


}
	
	
	
void mode2_circle_fill(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t color){
	int i=0;
	for (int i=0; i<radius; i++) {
		
		
		mode2_circle(x0,y0,radius-i,color);
		
	}
	

}




void mode2_render() {
    ili9341_start_writing();
	ili9341_write_data_continuous(mode2_buffer, SIZE*sizeof(uint16_t));
	ili9341_stop_writing();
}


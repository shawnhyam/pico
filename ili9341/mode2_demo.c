#include <stdint.h>
#include "mode2/mode2.h"
#include "ili9341/ili9341.h"

int main() {
	// possibly set ili9341_config parameters if your pins/spi port don't match
	ili9341_init();

	mode2_init();

	uint16_t x = 0;
	while (1) {
    	mode2_clear();
		mode2_rect(x, 40, 40, 80, 0xFFFF);
		mode2_render();

		x += 1;
		x = x%280;
	}
}


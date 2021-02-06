#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "mode0.h"
#include "ili9341.h"


int main() {
    mode0_init();
    
    mode0_set_cursor(0, 0);
    mode0_print("Hello world\n");
    mode0_print("This is more text\n");
    mode0_print("And here is more\n");
    
    while (1) {
        sleep_ms(10);
    }
}

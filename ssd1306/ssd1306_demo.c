#include "ssd1306.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"

int main() {



    sleep_ms(30);

	ssd1306_init(i2c0, 12, 13, 1984*1000);

    View view;
    view_init(&view);

    view_print(&view, "HELLO PICO\n");
    view_print(&view, "OLED 128x32 demo\n");
    view_print(&view, "Written in  C\n");
    view_render(&view);

    const uint LED_PIN = 25;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    while (true) {
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
    }
}

#include "ssd1306.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"

int main() {
    View view;



    sleep_ms(30);

    uint baud_rate = init_i2c();
    init_display();


    view_init(&view);

    view_print(&view, "HELLO PICO\n");
    view_print(&view, "OLED 128x32 demo\n");
    view_print(&view, "Written in  C++\n");
    view_render(&view);

//        int rate = baud_rate / 1000;
//    char thousands = rate / 1000 + 48;
//        char hundreds = (rate % 1000) / 100 + 48;
//        char tens = (rate % 100) / 10 + 48;
//        char ones = (rate % 10) + 48;
//        char str [] = { thousands, hundreds, tens, ones, 0 };
//        ssd1306_print(str);
//    ssd1306_print("\n");

//    show_scr(scr);
//    while (1) {
//        sleep_ms(1000);
//        for (uint8_t y=0; y<8; y++) {
//            ssd1306_scroll_y(1);
//            show_scr(scr);
//            sleep_ms(15);
//        }
//        ssd1306_print("Written by Shawn Hyam\n");
//        show_scr(scr);
//    }

    //ssd1306_scroll_y(

    /*
    int x = 0;
    while (1) {
        //for (int y=0; y<16; y++) {
            for (int y=0; y<32; y++) {
                draw_pixel(x, y, 1);
            }
        //}
        show_scr();
        sleep_ms(1);

        for (int y=0; y<32; y++) {
            draw_pixel(x, y, 0);
        }

        x += 1;
        x = x % 128;
    }
     */

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

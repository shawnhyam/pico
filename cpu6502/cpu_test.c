#include <stdio.h>
#include <string.h>

#include "fake6502.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"
#include "minimal_computer.h"
#include "pico/multicore.h"
#include "pico/scanvideo.h"
#include "pico/scanvideo/composable_scanline.h"
#include "pico/stdlib.h"
#include "vga/vga.h"

int main() {
    set_sys_clock_201_6mhz();
    stdio_init_all();

    measure_freqs();

    memcpy(mode0_charmem, cerberus_chardefs, 2048);

    for (int i = 0; i < 1200; i++) {
        mode0_vidmem[i] = i % 256;
    }

    // launch all the video on core 1
    multicore_launch_core1(mode0_loop);

    // wait for initialization of video to be complete
    // sem_acquire_blocking(&video_initted);
    sleep_ms(1000);

    getchar_timeout_us(0);

    // array size is 534
    static const uint8_t CELL6502[] = {
        0xa9, 0x80, 0x8d, 0x01, 0x00, 0x8d, 0x03, 0x00, 0x8d, 0x05, 0x00, 0x8d, 0x07, 0x00, 0xa9,
        0x20, 0xa0, 0x00, 0x99, 0x00, 0x80, 0xc8, 0xc0, 0x28, 0xd0, 0xf8, 0xa9, 0x08, 0xa0, 0x14,
        0x99, 0x00, 0x80, 0x20, 0xfc, 0x02, 0xa2, 0x00, 0x8e, 0x00, 0x82, 0x20, 0xaa, 0x02, 0xa0,
        0x00, 0xa9, 0x27, 0x8d, 0x00, 0x00, 0xad, 0x27, 0x80, 0x18, 0x6d, 0x00, 0x80, 0x6d, 0x01,
        0x80, 0x20, 0xbc, 0x02, 0x8d, 0x28, 0x80, 0xa9, 0x00, 0x8d, 0x00, 0x00, 0xa9, 0x01, 0x8d,
        0x02, 0x00, 0xa9, 0x29, 0x8d, 0x04, 0x00, 0xa9, 0x02, 0x8d, 0x06, 0x00, 0xb1, 0x00, 0x18,
        0x71, 0x02, 0x71, 0x06, 0x20, 0xbc, 0x02, 0x91, 0x04, 0xee, 0x00, 0x00, 0xee, 0x02, 0x00,
        0xee, 0x04, 0x00, 0xee, 0x06, 0x00, 0xad, 0x02, 0x00, 0xc9, 0x27, 0xd0, 0xe1, 0x20, 0xd6,
        0x02, 0x20, 0xe7, 0x02, 0x20, 0xfc, 0x02, 0xad, 0x00, 0x82, 0x18, 0x69, 0x08, 0x8d, 0x00,
        0x82, 0xc9, 0xd0, 0xd0, 0x05, 0xa9, 0x00, 0x8d, 0x00, 0x82, 0xad, 0x00, 0x02, 0xc9, 0x01,
        0xd0, 0x91, 0xa9, 0x00, 0x8d, 0x00, 0x02, 0xad, 0x01, 0x02, 0xc9, 0x73, 0xd0, 0x85, 0xcb,
        0x4c, 0x2b, 0x02, 0xae, 0x00, 0x82, 0xa0, 0x00, 0xbd, 0x48, 0x03, 0x99, 0x40, 0xf0, 0xe8,
        0xc8, 0xc0, 0x08, 0xd0, 0xf4, 0x60, 0xa0, 0x00, 0xc9, 0x60, 0xf0, 0x0e, 0xc9, 0x18, 0xf0,
        0x0a, 0xc9, 0x48, 0xf0, 0x09, 0xb1, 0x00, 0xc9, 0x20, 0xf0, 0x03, 0xa9, 0x20, 0x60, 0xa9,
        0x08, 0x60, 0xad, 0x26, 0x80, 0x18, 0x6d, 0x27, 0x80, 0x6d, 0x00, 0x80, 0x20, 0xbc, 0x02,
        0x8d, 0x4f, 0x80, 0x60, 0xa2, 0x28, 0xa0, 0x00, 0xbd, 0x00, 0x80, 0x99, 0x00, 0x80, 0xc8,
        0xe8, 0xe0, 0x28, 0xd0, 0xf4, 0xa2, 0x00, 0xa0, 0x00, 0x60, 0xa9, 0x00, 0x8d, 0x00, 0x00,
        0xa9, 0xf8, 0x8d, 0x01, 0x00, 0xa0, 0x28, 0xb1, 0x00, 0xa0, 0x00, 0x91, 0x00, 0xa0, 0x28,
        0x20, 0x34, 0x03, 0xad, 0x01, 0x00, 0xc9, 0xfc, 0xd0, 0xee, 0xad, 0x00, 0x00, 0xc9, 0x88,
        0xd0, 0xe7, 0xa9, 0x80, 0x8d, 0x01, 0x00, 0xa2, 0x00, 0xbd, 0x00, 0x80, 0x9d, 0x88, 0xfc,
        0xe8, 0xe0, 0x28, 0xd0, 0xf5, 0x60, 0xad, 0x00, 0x00, 0xc9, 0xff, 0xf0, 0x04, 0xee, 0x00,
        0x00, 0x60, 0xa9, 0x00, 0x8d, 0x00, 0x00, 0xee, 0x01, 0x00, 0x60, 0x00, 0x00, 0x00, 0x18,
        0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x3c, 0x3c, 0x18, 0x00, 0x00, 0x00, 0x18, 0x3c,
        0x7e, 0x7e, 0x3c, 0x18, 0x00, 0x18, 0x3c, 0x7e, 0xff, 0xff, 0x7e, 0x3c, 0x18, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x7f, 0x7f, 0x7f, 0xff, 0xff, 0xff, 0xff,
        0x00, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x00, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
        0x0f, 0x00, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x00, 0x00, 0x7f, 0x7f, 0x7f, 0x7e, 0x7e,
        0x7e, 0x00, 0x00, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x00, 0x00, 0x70, 0x7e, 0x7e, 0x7e,
        0x7e, 0x7e, 0x00, 0x00, 0x00, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x00, 0x00, 0x00, 0x3e, 0x3e,
        0x7e, 0x7e, 0x7e, 0x00, 0x00, 0x00, 0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x00, 0x00, 0x00, 0x3e,
        0x3e, 0x3e, 0x3e, 0x0e, 0x00, 0x00, 0x00, 0x3e, 0x3e, 0x3e, 0x3e, 0x00, 0x00, 0x00, 0x00,
        0x3e, 0x3e, 0x3c, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x3c, 0x3c, 0x3c, 0x00, 0x00, 0x00,
        0x00, 0x30, 0x3c, 0x3c, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x3c, 0x3c, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x1c, 0x1c, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x1c, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00};

    // Load a ROM into the mem array...
    memcpy(mem + 0x0202, CELL6502, sizeof(CELL6502));

    // set the reset vector
    mem[0xfffc] = 0x02;
    mem[0xfffd] = 0x02;

#if 0
        unsigned char str[40];
        size_t readLine() {
            unsigned char u, *p;
            for (p = str, u = getchar(); u != '\r' && p - str < 39; u = getchar()) {
                putchar(*p++ = u);
            }
            *p++ = 0;
            return p - str;
        }

        // there seems to always be one character in the input? so I guess we'll clear it...
        getchar_timeout_us(100);

        while (1) {
            printf("> ");
            stdio_flush();
            size_t size = readLine();
            printf("\nRead %d bytes: %s\n", size, str);
        }

        /*
         char action[4];
         int addr;
         int n=1;

         while (1) {
             int count = 0;
             int a[100];
        // Perform a do-while loop
         do {

             // Take input at position count
             // and increment count
             scanf("%d", &a[count++]);
             printf("C: %d %d\n", count, getchar());


             // If '\n' (newline) has occurred
             // or the whole array is filled,
             // then exit the loop

             // Otherwise, continue
         } while (getchar() != '\n' && count < 100);
             printf("STOP\n");

             int result = scanf("%s %x %d", action, &addr, &n);

             //printf("%d\n", result);
             printf("%04x: ", addr);
             for (int i=0; i<n; i++) {
                 printf("%02x ", mem[addr++]);
             }
             printf("\n");

             if (!strcmp(action, "r")) {
                 // read6502(addr);
             }
         }
         */
#endif

    reset6502();

    while (1) {
        // exec6502(10);
        // int ch = getchar_timeout_us(1000);
        // if (ch != PICO_ERROR_TIMEOUT) {
        //     // mem[0xff] = ch;
        // }
        uint32_t t0 = time_us_32();
        exec6502(10000000);
        uint32_t t1 = time_us_32();
        printf("time: %d\n", t1 - t0);
    }
}
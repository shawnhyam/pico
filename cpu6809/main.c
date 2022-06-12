/*
 * e6809 for Raspberry Pi Pico
 *
 * @version     1.0.0
 * @author      smittytone
 * @copyright   2022
 * @licence     MIT
 *
 */
#include "main.h"


/*
 *  GLOBALS
 */
uint8_t interrupts[3] = {PIN_6809_NMI, PIN_6809_IRQ, PIN_6809_FIRQ};


/*
 * ENTRY POINT
 */

/*
int main() {
    // Enable STDIO
    stdio_usb_init();
    #ifdef DEBUG
    // Pause to allow the USB path to initialize
    sleep_ms(2000);
    #endif

    // Prepare the board
    bool is_using_monitor = init_board();

    // Boot the CPU
    boot_cpu();

    // Branch according to whether the Pico is connected to a
    // monitor board or not (in which case run tests for now)
    if (is_using_monitor) {
        // Enter the monitor UI
        #ifdef DEBUG
        printf("Using monitor board\n");
        #endif

        monitor_event_loop();
    } else {
        // Run tests -- for now
        #ifdef DEBUG
        printf("Running tests\n");
        #endif

        test_main();
    }

    return 0;
}
*/

/**
 * @brief Bring up the virtual 6809e and 64KB of memory.
 *        In future, this will offer alternative memory layouts.
 */
void boot_cpu() {
    #ifdef DEBUG
    printf("Resetting the registers\n");
    #endif

    // Interrupt Vectors:
    uint16_t vectors[] = {0x0400,       //   Restart
                          0x0500,       //   NMI
                          0x0500,       //   SWI
                          0x0500,       //   IRQ
                          0x0500,       //   FIRQ
                          0x0500,       //   SWI2
                          0x0500,       //   SWI3
                          0x0000};      //   Reserved
    init_vectors(vectors);
    init_cpu();

    #ifdef DEBUG
    printf("Initializing memory\n");
    #endif
    for (uint16_t i = 0 ; i < START_VECTORS ; ++i) {
        mem[i] = RTI;
    }

    #ifdef DEBUG
    printf("Entering sample program at 0x4000\n");

    uint16_t start = 0x4000;
    uint8_t prog[] = {0x86,0xFF,0x8E,0x80,0x00,0xA7,0x80,0x8C,0x80,0x09,0x2D,0xF9,0x8E,0x40,0x00,0x6E,0x84};
    for (uint16_t i = 0 ; i < 17 ; i++) mem[start + i] = prog[i];

    reg.pc = 0x4000;
    reg.cc = 0x6B;

    /*
     * TEST PROGS
     */

    /*
    {0x34,0x46,0x33,0x64,0xA6,0x42,0xAE,0x43,0xE6,0x80,0x34,0x04,0x34,0x04,0x4A,0x27,0x13,0xE6,0x80,0xE1,0xE4,0x2E,0x08,0xE1,0x61,0x2E,0x06,0xE7,0x61,0x20,0x02,0xE7,0xE4,0x4A,0x26,0xED,0xA6,0xE0,0xA7,0x45,0xA6,0xE0,0xA7,0x46,0x35,0xC6,0x32,0x7E,0x8E,0x80,0x42,0x34,0x10,0xB6,0x80,0x41,0x34,0x02,0x8D,0xC4,0xA6,0x63,0xE6,0x64,0x39,0x0A,0x01,0x02,0x03,0x04,0x00,0x06,0x07,0x09,0x08,0x0B};
    {0x86, 0x41, 0x8E, 0x04, 0x00, 0xA7, 0x80, 0x8C, 0x06, 0x00, 0x26, 0xF9, 0x1A, 0x0F, 0x3B};
    */
    #endif

    #ifdef DEBUG
    printf("Setting interrupt vectors\n");
    #endif
    // Set up interrupt pins
    // TODO Keep here or place in CPU file?
    // for (uint8_t i = 0 ; i < 3 ; ++i) {
    //     gpio_init(interrupts[i]);
    //     gpio_set_dir(interrupts[i], GPIO_IN);
    //     gpio_pull_down(interrupts[i]);
    // }

    // // Set up the LED
    // gpio_init(PIN_PICO_LED);
    // gpio_set_dir(PIN_PICO_LED, GPIO_OUT);
    // gpio_put(PIN_PICO_LED, false);
}


/**
 * @brief Sample the interrupt pins and return a bitfield.
 *        This will be called by the
 */
uint8_t sample_interrupts() {
    uint8_t irqs = 0;
    for (uint8_t i = 0 ; i < 3 ; ++i) {
        if (gpio_get(interrupts[i])) irqs |= (1 << i);
    }
    return irqs;
}


/**
 * @brief Flash the Pico LED.
 *
 * @param count: The number of blinks in the sequence.
 */
void flash_led(uint8_t count) {
    while (count > 0) {
        gpio_put(PIN_PICO_LED, true);
        sleep_ms(250);
        gpio_put(PIN_PICO_LED, false);
        sleep_ms(250);
        count--;
    }
}


/*
 * EXPERIMENTAL
 */
// void read_into_ram() {
//     // See https://kevinboone.me/picoflash.html?i=1
//     // 2MB Flash = 2,097,152
//     // Allow 1MB for app code, so start at
//     // XIP_BASE + 1,048,576
//     // RAM SIZE = 64KB = 65,536
//     char *p = (char *)XIP_BASE;
//     p += 1048576;

//     // Read 64KB from Flash into RAM
//     for (uint16_t i = 0 ; i < 65536 ; ++i) {
//         mem[i] = (uint8_t)(*p);
//     }
// }


// void save_ram() {
//     // See https://kevinboone.me/picoflash.html?i=1
//     uint32_t irqs = save_and_disable_interrupts();
//     flash_range_erase (RP2040_FLASH_DATA_START, RP2040_FLASH_DATA_SIZE);
//     flash_range_program (RP2040_FLASH_DATA_START, mem, RP2040_FLASH_DATA_SIZE);
//     restore_interrupts (irqs);
// }

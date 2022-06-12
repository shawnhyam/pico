/*
 * e6809 for Raspberry Pi Pico
 *
 * @version     1.0.0
 * @author      smittytone
 * @copyright   2022
 * @licence     MIT
 *
 */
#ifndef _E6809_HEADER_
#define _E6809_HEADER_


/*
 *      INCLUDES
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
// #include "pico/binary_info.h"
// #include "hardware/gpio.h"
// #include "hardware/i2c.h"
// #include "hardware/spi.h"
// #include "hardware/adc.h"
// #include "hardware/flash.h"
// #include "hardware/sync.h"
#include "ops.h"
#include "cpu.h"
// #include "cpu_tests.h"
// #include "keypad.h"
// #include "ht16k33.h"
// #include "monitor.h"
// #include "pia.h"


/*
 *      CONSTANTS
 */
#define PIN_6809_NMI                22
#define PIN_6809_IRQ                20
#define PIN_6809_FIRQ               21

#define PIN_PICO_LED                25

#define RP2040_FLASH_DATA_START     1048576
#define RP2040_FLASH_DATA_SIZE      65536


/*
 *      PROTOTYPES
 */
void        boot_cpu();
uint8_t     sample_interrupts();
void        flash_led(uint8_t count);
void        run_tests();

// EXPERIMENTAL
void        read_into_ram();
void        save_ram();

/*
 * GLOBALS
 */
extern      REG_6809    reg;
extern      uint8_t     mem[KB64];
extern      STATE_6809  state;


#endif // _E6809_HEADER_

#ifndef _VGA_HEADER_
#define _VGA_HEADER_

#include <stdint.h>
#include <stdio.h>

#include "pico/multicore.h"

extern semaphore_t video_initted;

void set_sys_clock_201_6mhz(void);
void measure_freqs(void);

// MODE 0: Cerberus 2080 style video; two memory locations must be specified. There must be a 2kB
// region of character memory, and 1200 byte video memory region
extern uint8_t *mode0_charmem;
extern uint8_t *mode0_vidmem;
void mode0_loop(void);

// MODE 1: 80x40 text mode of characters that are 8x12. There must be a 3kB region of character
// memory, and 3200 byte video memory region
extern uint8_t *mode1_charmem;
extern uint8_t *mode1_vidmem;
void mode1_loop(void);

// MODE 2: Good old CoCo text mode, so we have a 32x16 video memory at 1 byte per block (512 bytes),
// plus a character memory of 128 characters that are 8x12 (1536 bytes)... could round up to 2kB?
// Consider supporting semigraphics modes.
extern uint8_t *mode2_charmem;
extern uint8_t *mode2_vidmem;
void mode2_loop(void);

// MODE X: Unknown what this is -- some VGA experiments I guess
void modex_loop(void);

// MODE Z: generate video that matches the easy6502 specification
// modez_mem needs to be set before calling modez_loop; should be 32*32 bytes and represents the
// contents of video memory, 32x32 pixels at 4bpp (but stored in 1 byte)
extern uint8_t *modez_mem;
void modez_loop(void);

// Font Definitions
extern const uint8_t CLGD5442_8x8[2048];
extern const uint8_t CLGD5442_8x14[3584];
extern const uint8_t CLGD5442_8x16[4096];
extern const uint8_t cerberus_chardefs[2048];
extern const uint8_t coco_font[1152];

#endif

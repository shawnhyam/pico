#ifndef _MINIMALCOMPUTER_H_
#define _MINIMALCOMPUTER_H_

#include <stdint.h>
#include <stdio.h>

#include "pico/scanvideo.h"

extern uint8_t mem[65536];
extern const scanvideo_mode_t my_mode;
extern uint16_t palette[16];

uint8_t read6502(uint16_t address);
void write6502(uint16_t address, uint8_t value);

#endif

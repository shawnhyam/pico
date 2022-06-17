#ifndef _MINIMALCOMPUTER_H_
#define _MINIMALCOMPUTER_H_

#include <stdint.h>
#include <stdio.h>

extern uint8_t mem[65536];

uint8_t read6502(uint16_t address);
void write6502(uint16_t address, uint8_t value);

#endif


#include "minimal_computer.h"

#include "hardware/regs/addressmap.h"
#include "hardware/regs/rosc.h"

uint8_t mem[65536];
uint8_t* modez_mem = mem + 0x200;

uint8_t* mode0_charmem = mem + 0xf000;
uint8_t* mode0_vidmem = mem + 0xf800;

uint32_t rnd(void) {
    int k, random = 0;
    volatile uint32_t* rnd_reg = (uint32_t*)(ROSC_BASE + ROSC_RANDOMBIT_OFFSET);

    for (k = 0; k < 32; k++) {
        random = random << 1;
        random = random + (0x00000001 & (*rnd_reg));
    }
    return random;
}

uint8_t read6502(uint16_t address) {
    // printf("Read  %04x :  %02x\n", address, mem[address]);
    if (address == 0xfe) {
        return rnd() % 256;
    }
    return mem[address];
}

void write6502(uint16_t address, uint8_t value) {
    // printf("Write %04x -> %02x\n", address, value);
    mem[address] = value;
}

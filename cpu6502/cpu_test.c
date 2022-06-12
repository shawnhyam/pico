#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
// #include "vrEmu6502.h"

extern void reset6502();  
extern void step6502();
extern void exec6502(uint32_t tickcount);
extern void irq6502();
extern void nmi6502();
extern void hookexternal(void *funcptr);

uint8_t mem[65536];

uint8_t read6502(uint16_t address) {
    printf("Read  %04x :  %02x\n", address, mem[address]);
    return mem[address];
}

void write6502(uint16_t address, uint8_t value) {
    printf("Write %04x -> %02x\n", address, value);
    mem[address] = value;
}
 
int main() {
    stdio_init_all();
    printf("Starting up...\n");

    // Load a ROM into the mem array...
    // from https://skilldrick.github.io/easy6502/
    // 0600: a9 01 8d 00 02 a9 05 8d 01 02 a9 08 8d 02 02 
    static const uint8_t program[] = { 0xa9, 0x01, 0x8d, 0x00, 0x02, 0xa9, 0x05, 0x8d, 0x01, 0x02, 0xa9, 0x08, 0x8d, 0x02, 0x02 };
    memcpy(mem+0x0600, program, sizeof(program));

    mem[0xfffc] = 0x00;
    mem[0xfffd] = 0x06;

    // TODO generate video that matches the easy6502 specification
    // NOTE more programs at https://github.com/r00ster91/easy6502
    


    reset6502();
    for (int i=0; i<10; i++) {
        step6502();
    }


    while (1) {}
}
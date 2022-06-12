#include <stdio.h>
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

    // TODO Load a ROM into the mem array...

    reset6502();
    for (int i=0; i<10; i++) {
        step6502();
    }


    while (1) {}
}
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "ili9341.h"

/*
 
 (pin 1) VCC        5V/3.3V power input
 (pin 2) GND        Ground
 (pin 3) CS         LCD chip select signal, low level enable
 (pin 4) RESET      LCD reset signal, low level reset
 (pin 5) DC/RS      LCD register / data selection signal; high level: register, low level: data
 (pin 6) SDI(MOSI)  SPI bus write data signal
 (pin 7) SCK        SPI bus clock signal
 (pin 8) LED        Backlight control; if not controlled, connect 3.3V always bright
 (pin 9) SDO(MISO)  SPI bus read data signal; optional
 
 */


static inline void cs_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 1);
    asm volatile("nop \n nop \n nop");
}

void ili9341_set_command(uint8_t cmd) {
    cs_select();
    gpio_put(PIN_DC, 0);
    spi_write_blocking(SPI_PORT, &cmd, 1);
    gpio_put(PIN_DC, 1);
    cs_deselect();
}

void ili9341_command_param(uint8_t data) {
    cs_select();
    spi_write_blocking(SPI_PORT, &data, 1);
    cs_deselect();
}

void ili9341_write_data(void *buffer, int bytes) {
    cs_select();
    spi_write_blocking(SPI_PORT, buffer, bytes);
    cs_deselect();
}

uint16_t swap_bytes(uint16_t color) {
    return (color>>8) | (color<<8);
}


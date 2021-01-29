#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

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

#define PIN_MISO 4
#define PIN_CS   5
#define PIN_SCK  6
#define PIN_MOSI 7
#define PIN_RESET 8
#define PIN_DC 9  // data/command

#define SPI_PORT spi0


#define ILI9341_TFTWIDTH 240  ///< ILI9341 max TFT width
#define ILI9341_TFTHEIGHT 320 ///< ILI9341 max TFT height

#define ILI9341_NOP 0x00     ///< No-op register
#define ILI9341_SWRESET 0x01 ///< Software reset register
#define ILI9341_RDDID 0x04   ///< Read display identification information
#define ILI9341_RDDST 0x09   ///< Read Display Status

#define ILI9341_SLPIN 0x10  ///< Enter Sleep Mode
#define ILI9341_SLPOUT 0x11 ///< Sleep Out
#define ILI9341_PTLON 0x12  ///< Partial Mode ON
#define ILI9341_NORON 0x13  ///< Normal Display Mode ON

#define ILI9341_RDMODE 0x0A     ///< Read Display Power Mode
#define ILI9341_RDMADCTL 0x0B   ///< Read Display MADCTL
#define ILI9341_RDPIXFMT 0x0C   ///< Read Display Pixel Format
#define ILI9341_RDIMGFMT 0x0D   ///< Read Display Image Format
#define ILI9341_RDSELFDIAG 0x0F ///< Read Display Self-Diagnostic Result

#define ILI9341_INVOFF 0x20   ///< Display Inversion OFF
#define ILI9341_INVON 0x21    ///< Display Inversion ON
#define ILI9341_GAMMASET 0x26 ///< Gamma Set
#define ILI9341_DISPOFF 0x28  ///< Display OFF
#define ILI9341_DISPON 0x29   ///< Display ON

#define ILI9341_CASET 0x2A ///< Column Address Set
#define ILI9341_PASET 0x2B ///< Page Address Set
#define ILI9341_RAMWR 0x2C ///< Memory Write
#define ILI9341_RAMRD 0x2E ///< Memory Read

#define ILI9341_PTLAR 0x30    ///< Partial Area
#define ILI9341_VSCRDEF 0x33  ///< Vertical Scrolling Definition
#define ILI9341_MADCTL 0x36   ///< Memory Access Control
#define ILI9341_VSCRSADD 0x37 ///< Vertical Scrolling Start Address
#define ILI9341_PIXFMT 0x3A   ///< COLMOD: Pixel Format Set

#define ILI9341_FRMCTR1                                                        \
  0xB1 ///< Frame Rate Control (In Normal Mode/Full Colors)
#define ILI9341_FRMCTR2 0xB2 ///< Frame Rate Control (In Idle Mode/8 colors)
#define ILI9341_FRMCTR3                                                        \
  0xB3 ///< Frame Rate control (In Partial Mode/Full Colors)
#define ILI9341_INVCTR 0xB4  ///< Display Inversion Control
#define ILI9341_DFUNCTR 0xB6 ///< Display Function Control

#define ILI9341_PWCTR1 0xC0 ///< Power Control 1
#define ILI9341_PWCTR2 0xC1 ///< Power Control 2
#define ILI9341_PWCTR3 0xC2 ///< Power Control 3
#define ILI9341_PWCTR4 0xC3 ///< Power Control 4
#define ILI9341_PWCTR5 0xC4 ///< Power Control 5
#define ILI9341_VMCTR1 0xC5 ///< VCOM Control 1
#define ILI9341_VMCTR2 0xC7 ///< VCOM Control 2

#define ILI9341_RDID1 0xDA ///< Read ID 1
#define ILI9341_RDID2 0xDB ///< Read ID 2
#define ILI9341_RDID3 0xDC ///< Read ID 3
#define ILI9341_RDID4 0xDD ///< Read ID 4

#define ILI9341_GMCTRP1 0xE0 ///< Positive Gamma Correction
#define ILI9341_GMCTRN1 0xE1 ///< Negative Gamma Correction
//#define ILI9341_PWCTR6     0xFC

// Color definitions
#define ILI9341_BLACK 0x0000       ///<   0,   0,   0
#define ILI9341_NAVY 0x000F        ///<   0,   0, 123
#define ILI9341_DARKGREEN 0x03E0   ///<   0, 125,   0
#define ILI9341_DARKCYAN 0x03EF    ///<   0, 125, 123
#define ILI9341_MAROON 0x7800      ///< 123,   0,   0
#define ILI9341_PURPLE 0x780F      ///< 123,   0, 123
#define ILI9341_OLIVE 0x7BE0       ///< 123, 125,   0
#define ILI9341_LIGHTGREY 0xC618   ///< 198, 195, 198
#define ILI9341_DARKGREY 0x7BEF    ///< 123, 125, 123
#define ILI9341_BLUE 0x001F        ///<   0,   0, 255
#define ILI9341_GREEN 0x07E0       ///<   0, 255,   0
#define ILI9341_CYAN 0x07FF        ///<   0, 255, 255
#define ILI9341_RED 0xF800         ///< 255,   0,   0
#define ILI9341_MAGENTA 0xF81F     ///< 255,   0, 255
#define ILI9341_YELLOW 0xFFE0      ///< 255, 255,   0
#define ILI9341_WHITE 0xFFFF       ///< 255, 255, 255
#define ILI9341_ORANGE 0xFD20      ///< 255, 165,   0
#define ILI9341_GREENYELLOW 0xAFE5 ///< 173, 255,  41
#define ILI9341_PINK 0xFC18        ///< 255, 130, 198

#define LCD_W 320
#define LCD_H 240
#define SWAP_RED_BLUE 1  // my device didn't match the datasheet


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

uint16_t swap_red_blue(uint16_t color) {
    int r = (color & 0xf800) >> 11;
    int g = (color & 0x07d0) >> 5;
    int b = (color & 0x001f);
    
    return (b << 11) + (g << 5) + r;
}

int main() {
    stdio_init_all();

    // This example will use SPI0 at 0.5MHz.
    spi_init(SPI_PORT, 500 * 1000);
    int baudrate = spi_set_baudrate(SPI_PORT, 48000 * 1000);
    
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 0);
    
    // Reset is active-low
    gpio_init(PIN_RESET);
    gpio_set_dir(PIN_RESET, GPIO_OUT);
    gpio_put(PIN_RESET, 1);
    
    // high = command, low = data
    gpio_init(PIN_DC);
    gpio_set_dir(PIN_DC, GPIO_OUT);
    gpio_put(PIN_DC, 0);
    
    
    sleep_ms(10);
    gpio_put(PIN_RESET, 0);
    sleep_ms(10);
    gpio_put(PIN_RESET, 1);
    
    ili9341_set_command(0x01);//soft reset
    sleep_ms(100);
    
    // memory access control
    ili9341_set_command(ILI9341_MADCTL);
    ili9341_command_param(0x40);
    
    // pixel format
    ili9341_set_command(ILI9341_PIXFMT);
    ili9341_command_param(0x55);  // 16-bit
    
    // frame rate; default, 70 Hz
    ili9341_set_command(ILI9341_FRMCTR1);
    ili9341_command_param(0x00);
    ili9341_command_param(0x1B);

    // exit sleep
    ili9341_set_command(ILI9341_SLPOUT);
    
    // display on
    ili9341_set_command(ILI9341_DISPON);

    // column address set
    ili9341_set_command(ILI9341_CASET);
    ili9341_command_param(0x00);
    ili9341_command_param(0x00);  // start column
    ili9341_command_param(0x00);
    ili9341_command_param(0xef);  // end column -> 239

    ili9341_set_command(ILI9341_PASET);
    ili9341_command_param(0x00);
    ili9341_command_param(0x00);  // start page
    ili9341_command_param(0x01);
    ili9341_command_param(0x3f);  // end page -> 319

    ili9341_set_command(ILI9341_RAMWR);
    
    uint16_t colors[16] = {
        0x8000,
        0x4000,
        0x2000,
        0x1000,
        0x0800,
        0x0400,
        0x0200,
        0x0100,
        0x0080,
        0x0040,
        0x0020,
        0x0010,
        0x0008,
        0x0004,
        0x0002,
        0x0001
    };

    for (int z=0; z<8; z++) {
        uint16_t buf[240*40];
        for (int y=0; y<40; y++) {
            for (int a=0; a<16; a++) {
                uint16_t color = colors[(a+z) % 16];
#if SWAP_RED_BLUE
                color = swap_red_blue(color);
#endif
                // endianness of the devices don't match, swap the bytes
                color = (color>>8) | (color<<8);

                for (int x=0; x<15; x += 1) {
                    buf[y*240+a*15+x] = color;
                }
            }
        }
        //spi_write_blocking(SPI_PORT, (uint8_t*)buf, 240*40*2);
        ili9341_write_data(buf, 240*40*2);
    }
    
    
    uint16_t scroll = 0;
    while (1) {
        ili9341_set_command(ILI9341_VSCRSADD);
        ili9341_command_param((uint8_t)(scroll / 256));
        ili9341_command_param((uint8_t)(scroll % 256));
        scroll += 1;
        scroll = scroll % 320;
        
        sleep_ms(10);
    }

    const uint LED_PIN = 25;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // flash the LED and send SPI rate to the terminal
    uint8_t pin = 0;
    while (1) {
        printf("Baudrate: %d\n", baudrate);
        
        pin = 1-pin;
        gpio_put(LED_PIN, pin);
        sleep_ms(1000);
    }
}

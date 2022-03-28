#include "pico/stdlib.h"
#include "hardware/pio.h"
// #include "hardware/irq.h"
#include "hardware/dma.h"
// #include "hardware/gpio.h"
#include "hardware/structs/bus_ctrl.h"
#include <stdio.h>

// Our assembled PIO program
#include "vga_sync.pio.h"



#define vid_pio pio0

#define HSYNC_PIN 16
#define VSYNC_PIN 17
#define VID_PIN 0

#define PIXELS_PER_LINE 640
#define ACTIVE_LINES_PER_FRAME 479


uint dma_chan;
uint16_t line_buffer[PIXELS_PER_LINE];
uint16_t empty_line_buffer[PIXELS_PER_LINE];

void measure_freqs(void) {
    uint f_pll_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
    uint f_pll_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_USB_CLKSRC_PRIMARY);
    uint f_rosc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_ROSC_CLKSRC);
    uint f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
    uint f_clk_peri = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI);
    uint f_clk_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB);
    uint f_clk_adc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_ADC);
    uint f_clk_rtc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC);
 
    printf("pll_sys  = %dkHz\n", f_pll_sys);
    printf("pll_usb  = %dkHz\n", f_pll_usb);
    printf("rosc     = %dkHz\n", f_rosc);
    printf("clk_sys  = %dkHz\n", f_clk_sys);
    printf("clk_peri = %dkHz\n", f_clk_peri);
    printf("clk_usb  = %dkHz\n", f_clk_usb);
    printf("clk_adc  = %dkHz\n", f_clk_adc);
    printf("clk_rtc  = %dkHz\n", f_clk_rtc);
 
    // Can't measure clk_ref / xosc as it is the ref
}

uint h_line_count;

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == HSYNC_PIN) { //if horizontal pulse triggered, increment line count
		if (h_line_count >= 45) {
			dma_channel_set_read_addr(dma_chan, line_buffer, true) ;// start DMA
		} else {
            dma_channel_set_read_addr(dma_chan, empty_line_buffer, true) ;// start DMA
        }
		++h_line_count;
	}
	else	{  // vsync is the interrupt, "0" is the start of vert front porch
		h_line_count = 10; // vsync starts 10 lines after the front porch
        dma_channel_set_read_addr(dma_chan, empty_line_buffer, true) ;// start DMA
	}
}


int main() {
    // system clock of 201.6 MHz; almost exactly 8x VGA speed (25.2 MHz vs 25.175 MHz)
    // set_sys_clock_pll(1008000000, 5, 1);

    // system clock of 176.0 MHz; almost exactly 7x VGA speed (25.14 MHz vs 25.175 MHz)
    // set_sys_clock_pll(1584000000, 3, 3);

    // system clock of 151.2 MHz; almost exactly 6x VGA speed (25.2 MHz vs 25.175 MHz)
    // set_sys_clock_pll(1512000000, 5, 2);

    // system clock of 126 MHz; almost exactly 5x VGA speed (25.2 MHz vs 25.175 MHz)
    set_sys_clock_pll(1512000000, 6, 2);

    // system clock of 100.8 MHz; almost exactly 4x VGA speed (25.175 MHz vs 25.175 MHz)
    // set_sys_clock_pll(1512000000, 5, 3);

	stdio_init_all();
    measure_freqs(); 
 
 	bus_ctrl_hw->priority = BUSCTRL_BUS_PRIORITY_DMA_W_BITS | BUSCTRL_BUS_PRIORITY_DMA_R_BITS;

	// Our assembled programs needs to be loaded into this PIO's instruction
    // memory. This SDK function will find a location (offset) in the
    // instruction memory where there is enough space for our program. We need
    // to remember this location!
    uint hoffset = pio_add_program(vid_pio, &video_hsync_program);
	uint voffset = pio_add_program(vid_pio, &video_vsync_program);
	uint vidoffset = pio_add_program(vid_pio, &vid_out_program);
	
	// select the desired state machine clock frequency 
	float SMv_CLK_FREQ = 25175000;   // vsync
	float SMh_CLK_FREQ = 25175000 / 16.0;   // hsync
	float SMvid_CLK_FREQ = 126000000; //vid_out  -- need to match number above


    // Find a free state machine on our chosen PIO (erroring if there are
    // none). Configure it to run our programs, and start it, using the
    // helper functions we included in our .pio file.
    uint smh = pio_claim_unused_sm(vid_pio, true);
	uint smv = pio_claim_unused_sm(vid_pio, true);
	uint smvid = pio_claim_unused_sm(vid_pio, true);

    // initialize the VGA PIO
	video_hsync_program_init(vid_pio, smh, hoffset, HSYNC_PIN, SMh_CLK_FREQ);
	video_vsync_program_init(vid_pio, smv, voffset, VSYNC_PIN, SMv_CLK_FREQ);
	vid_out_program_init(vid_pio, smvid, vidoffset, VID_PIN, SMvid_CLK_FREQ);

    // All state machines are now running.

    // put the counter value for vsync into the 'x' register of the vsync state machine 
	pio_sm_put (vid_pio, smv,  ACTIVE_LINES_PER_FRAME); // counter value for vsync
	pio_sm_exec(vid_pio, smv, pio_encode_pull(false, false));
	pio_sm_exec(vid_pio, smv, pio_encode_out(pio_x,32));

	// Identify the interupt states
	gpio_set_irq_enabled_with_callback(HSYNC_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
	gpio_set_irq_enabled(VSYNC_PIN, GPIO_IRQ_EDGE_FALL, true);

	//Set-up the DMA channel to transmit the line_buffer
    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
    channel_config_set_dreq(&c,  pio_get_dreq(vid_pio, smvid, true)); //This sets up the dreq for sending data to sm
	
	dma_channel_configure(
        dma_chan,
        &c,
        &pio0_hw->txf[smvid], // Write address (smvid transmit FIFO)
        NULL,      // read pixel data from line buffer
        PIXELS_PER_LINE,  // Write a line of pixels, then halt and interrupt
        false            //  don't start yet
    );

/*
setup for the test pattern:
Red is 0 to 31; 31 is pure red
green is 64 to 2047; 1984 is pure green
blue is 2048 to 65535; 63488 is pure blue
cyan = 65472, 63519 is magenta,65535 is white, 2015 is yellow
*/

	uint color[641] = {
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0,
        31,1984,63488,65535,2015,63519,65477,0,   31,1984,63488,65535,2015,63519,65477,0
     };
    //uint color[8] = { 0, 31, 31, 0, 0, 31, 31, 0 };
	uint array_counter;
    for (uint array_counter = 0; array_counter < PIXELS_PER_LINE; array_counter++) {
        line_buffer[array_counter] = color[array_counter];
        empty_line_buffer[array_counter] = 0;
    }

	while(true) {		//idle while DMA and PIO works
		//sleep_ms(10000);		//for test purposes
 		// tight_loop_contents	;
	}
}


#include "pico/stdlib.h"
#include "hardware/pio.h"
// #include "hardware/irq.h"
// #include "hardware/dma.h"
// #include "hardware/gpio.h"
// #include "hardware/structs/bus_ctrl.h"
#include <stdio.h>
// Our assembled PIO program:

int main() {
	stdio_init_all();
}


#if 0
#define vid_pio pio0
#define PIXELS_PER_LINE 640
#define ACTIVE_LINES_PER_FRAME 487

uint HSYNC_PIN = 16;
uint VSYNC_PIN = 17;
uint VID_PIN = 0;
uint h_line_count;
uint dma_chan;
uint16_t line_buffer[PIXELS_PER_LINE];
 
void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == HSYNC_PIN) { //if horizontal pulse triggered, increment line count
		if (h_line_count >= 45) {
			dma_channel_set_read_addr(dma_chan,line_buffer,true) ;// start DMA
		}
		++h_line_count;
	}
	else	{  // vsync is the interrupt, "0" is the start of vert front porch
		h_line_count = 10; // vsync starts 10 lines after the front porch
	}
}

int main() {
	stdio_init_all();  // set up to print out
	
	bus_ctrl_hw->priority = BUSCTRL_BUS_PRIORITY_DMA_W_BITS | BUSCTRL_BUS_PRIORITY_DMA_R_BITS;
	
	
	// Our assembled programs needs to be loaded into this PIO's instruction
    // memory. This SDK function will find a location (offset) in the
    // instruction memory where there is enough space for our program. We need
    // to remember this location!
    uint hoffset = pio_add_program(vid_pio, &video_hsync_program);
	uint voffset = pio_add_program(vid_pio, &video_vsync_program);
	uint vidoffset = pio_add_program(vid_pio, &vid_out_program);
	
	
	// select the desired state machine clock frequency 
	float SMh_CLK_FREQ = 1562500 ;   //hsync
	float SMv_CLK_FREQ = 25000000;   //vsync
	float SMvid_CLK_FREQ = 125000000; //vid_out

    // Find a free state machine on our chosen PIO (erroring if there are
    // none). Configure it to run our programs, and start it, using the
    // helper functions we included in our .pio file.
    uint smh = pio_claim_unused_sm(vid_pio, true);
	uint smv = pio_claim_unused_sm(vid_pio, true);
	uint smvid = pio_claim_unused_sm(vid_pio, true);
	
	video_hsync_program_init(vid_pio, smh, hoffset, HSYNC_PIN, SMh_CLK_FREQ);
	video_vsync_program_init(vid_pio, smv, voffset, VSYNC_PIN, SMv_CLK_FREQ);
	vid_out_program_init(vid_pio, smvid, vidoffset, VID_PIN, SMvid_CLK_FREQ);

    // All state machines are now running. 
	pio_sm_put (vid_pio, smv,  ACTIVE_LINES_PER_FRAME); // counter value for vsync
	pio_sm_exec(vid_pio, smv, pio_encode_pull(false, false));
	pio_sm_exec(vid_pio, smv, pio_encode_out(pio_x,32));

	//Identify the interupt states
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
        line_buffer,      // read pixel data from line buffer
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

	uint color[8] = {31,1984,63488,0,2015,63519,65477,65535};
	uint array_counter;

		for (int i = 0; i < 8; ++i){ 				// write 8 colors across the screen
			for (int j = 0; j < 80; ++j){
				array_counter = (i*80) + j;
				line_buffer[array_counter] = color[i];
			}
		}
//		Clearing the end of line_buffer for testing purposes
		for (int i = 620; i < PIXELS_PER_LINE; ++i){
			line_buffer[i] = 0;
		}
//		Clearing the beginning of line buffer for testing purposes		
		for (int i = 0; i < 2; ++i){
			line_buffer[i] = 0;
		}
	while(true) {		//idle while DMA and PIO works
//		sleep_ms(10000);		//for test purposes
 		tight_loop_contents	;
	}
}
#endif

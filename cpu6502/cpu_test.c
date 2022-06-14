#include <stdio.h>
#include <string.h>
#include "hardware/clocks.h"
#include "hardware/irq.h"

#include "pico/stdlib.h"
// #include "vrEmu6502.h"
#include "pico/multicore.h"
#include "pico/scanvideo.h"
#include "pico/scanvideo/composable_scanline.h"
#include "hardware/regs/rosc.h"
#include "hardware/regs/addressmap.h"
static semaphore_t video_initted;

extern void reset6502();  
extern void step6502();
extern void exec6502(uint32_t tickcount);
extern void irq6502();
extern void nmi6502();
extern void hookexternal(void *funcptr);


const uint startLineLength = 8; // the linebuffer will automatically grow for longer lines
const char eof = 255;           // EOF in stdio.h -is -1, but getchar returns int 255 to avoid blocking

/*
 *  read a line of any  length from stdio (grows)
 *
 *  @param fullDuplex input will echo on entry (terminal mode) when false
 *  @param linebreak defaults to "\n", but "\r" may be needed for terminals
 *  @return entered line on heap - don't forget calling free() to get memory back
 */
size_t getLine(char* buffer, size_t len, bool fullDuplex) {
    // th line buffer
    // will allocated by pico_malloc module if <cstdlib> gets included
    //char * pStart = (char*)malloc(startLineLength); 
    char * pPos = buffer;  // next character position
    int c;

    while(1) {
        c = getchar_timeout_us(100000000); // expect next character entry

        if(c == '\n') {
            break;     // non blocking exit
        }

        if (fullDuplex) {
            putchar(c); // echo for fullDuplex terminals
        }

        if(--len == 0) { // allow larger buffer
            return pPos - buffer;
        }

        // stop reading if lineBreak character entered 
        if((*pPos++ = c) == '\n') {
            break;
        }
    }

    *pPos = '\0';   // set string end mark
    return pPos-buffer;
}


uint32_t rnd(void){
    int k, random=0;
    volatile uint32_t *rnd_reg=(uint32_t *)(ROSC_BASE + ROSC_RANDOMBIT_OFFSET);
    
    for(k=0;k<32;k++){
    
    random = random << 1;
    random=random + (0x00000001 & (*rnd_reg));

    }
    return random;
}
uint8_t mem[65536];

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

const scanvideo_timing_t my_vga_mode_640x480_60 =
        {
                .clock_freq = 25200000,

                .h_active = 640,
                .v_active = 480,

                .h_front_porch = 16,
                .h_pulse = 64,
                .h_total = 800,
                .h_sync_polarity = 1,

                .v_front_porch = 1,
                .v_pulse = 2,
                .v_total = 525,
                .v_sync_polarity = 1,

                .enable_clock = 0,
                .clock_polarity = 0,

                .enable_den = 0
        };

const scanvideo_mode_t my_mode = {
    .default_timing = &my_vga_mode_640x480_60,
    .pio_program = &video_24mhz_composable,
    .width = 640,
    .height = 480,
    .xscale = 1,
    .yscale = 1,
    .yscale_denominator = 1
};

uint16_t palette[16] = {
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x00, 0x00),  // 000000
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x1d, 0x2b, 0x53),  // 1D2B53
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x7e, 0x25, 0x53),  // 7E2553
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x87, 0x51),  // 008751
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xab, 0x52, 0x36),  // AB5236
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x5f, 0x57, 0x4f),  // 5F574F
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xc2, 0xc3, 0xc7),  // C2C3C7
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xff, 0xf1, 0xe8),  // FFF1E8
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xff, 0x00, 0x4d),  // FF004D
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xff, 0xa3, 0x00),  // FFA300
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xff, 0xec, 0x27),  // FFEC27
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0xe4, 0x36),  // 00E436
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x29, 0xad, 0xff),  // 29ADFF
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x83, 0x76, 0x9c),  // 83769C
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xff, 0x77, 0xa8),  // FF77A8
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xff, 0xcc, 0xaa)   // FFCCAA

};

//uint8_t framebuffer[32*32];

void draw_scanline_fragment(scanvideo_scanline_buffer_t *buffer) {
    // figure out 1/32 of the color value
    uint line_num = scanvideo_scanline_number(buffer->scanline_id);
    uint frame_num = scanvideo_frame_number(buffer->scanline_id);

    if ((line_num == 0) && (frame_num % 60) == 0) {
        // printf("tick...\n");
    }

    uint16_t *p = (uint16_t *)buffer->data;
        uint v = line_num / 15;

        *p++ = COMPOSABLE_COLOR_RUN;
        *p++ = palette[1]; //PICO_SCANVIDEO_PIXEL_FROM_RGB8(0, 0, 0);
        *p++ = 77;

        for (int x=0; x<32; x++) {
            uint color = mem[0x0200 + x+v*32];  // (x+v)%16

            *p++ = COMPOSABLE_COLOR_RUN;
            *p++ = palette[color % 16];
            *p++ = 12;
        }

        *p++ = COMPOSABLE_COLOR_RUN;
        *p++ = palette[1]; //PICO_SCANVIDEO_PIXEL_FROM_RGB8(0, 0, 0);
        *p++ = 77;


    *p++ = COMPOSABLE_RAW_1P;
    *p++ = 0;
    *p++ = COMPOSABLE_EOL_ALIGN;

    buffer->data_used = ((uint32_t *)p) - buffer->data;
    buffer->status = SCANLINE_OK;

}


void core1_func() {
    // initialize video and interrupts on core 1
    scanvideo_setup(&my_mode);

    scanvideo_timing_enable(true);
    sem_release(&video_initted);

    // how can we support a configurable background color, behind layer 1?

    while (true) {
        scanvideo_scanline_buffer_t *scanline_buffer = scanvideo_begin_scanline_generation(true);
        draw_scanline_fragment(scanline_buffer);
        scanvideo_end_scanline_generation(scanline_buffer);


        // can we trigger some work after the last scanline of a frame?
        // what is the best way to trigger work on core 1 during VSYNC? what work would we perform?
        // updating the frame data makes sense...
    }
}
 
 // TODO move this into a helper library?
void measure_freqs(void) {
    uint f_pll_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
    uint f_pll_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_USB_CLKSRC_PRIMARY);
    uint f_rosc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_ROSC_CLKSRC);
    uint f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
    uint f_clk_peri = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI);
    uint f_clk_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB);
    uint f_clk_adc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_ADC);
    uint f_clk_rtc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC);
    uint sys_clk = clock_get_hz(clk_sys);

    clock_set_reported_hz(clk_sys, sys_clk);

 
    printf("pll_sys  = %dkHz\n", f_pll_sys);
    printf("pll_usb  = %dkHz\n", f_pll_usb);
    printf("rosc     = %dkHz\n", f_rosc);
    printf("clk_sys  = %dkHz\n", f_clk_sys);
    printf("clk_peri = %dkHz\n", f_clk_peri);
    printf("clk_usb  = %dkHz\n", f_clk_usb);
    printf("clk_adc  = %dkHz\n", f_clk_adc);
    printf("clk_rtc  = %dkHz\n", f_clk_rtc); 
    printf("sys_clk  = %dHz\n\n", sys_clk); 
}

int main() {
    // scanvideo library wants specific clock rates (integer multiples of 25MHz?)
    // pico-sdk/src/rp2_common/hardware_clocks/scripts/vcocalc.py 201.4

    // Requested: 201.4 MHz
    // Achieved: 201.6 MHz
    // FBDIV: 84 (VCO = 1008 MHz)
    // PD1: 5
    // PD2: 1
    set_sys_clock_pll(1008000000, 5, 1);
    stdio_init_all();
    printf("Starting up...\n");
    measure_freqs();
 
    // launch all the video on core 1, so it isn't affected by USB handling on core 0
    multicore_launch_core1(core1_func);

    // wait for initialization of video to be complete
    // sem_acquire_blocking(&video_initted);

    // getchar_timeout_us(0);

    // Load a ROM into the mem array...
    // from https://skilldrick.github.io/easy6502/
    // 0600: a9 01 8d 00 02 a9 05 8d 01 02 a9 08 8d 02 02 
    // static const uint8_t program[] = { 0xa9, 0x01, 0x8d, 0x00, 0x02, 0xa9, 0x05, 0x8d, 0x01, 0x02, 0xa9, 0x08, 0x8d, 0x02, 0x02 };


    static const uint8_t program[] = { 
        0x20, 0x06, 0x06, 0x20, 0x38, 0x06, 0x20, 0x0d, 0x06, 0x20, 0x2a, 0x06, 0x60, 0xa9, 0x02, 0x85,
        0x02, 0xa9, 0x04, 0x85, 0x03, 0xa9, 0x11, 0x85, 0x10, 0xa9, 0x10, 0x85, 0x12, 0xa9, 0x0f, 0x85,
        0x14, 0xa9, 0x04, 0x85, 0x11, 0x85, 0x13, 0x85, 0x15, 0x60, 0xa5, 0xfe, 0x85, 0x00, 0xa5, 0xfe,
0x29, 0x03, 0x18, 0x69, 0x02, 0x85, 0x01, 0x60, 0x20, 0x4d, 0x06, 0x20, 0x8d, 0x06, 0x20, 0xc3,
0x06, 0x20, 0x19, 0x07, 0x20, 0x20, 0x07, 0x20, 0x2d, 0x07, 0x4c, 0x38, 0x06, 0xa5, 0xff, 0xc9,
0x77, 0xf0, 0x0d, 0xc9, 0x64, 0xf0, 0x14, 0xc9, 0x73, 0xf0, 0x1b, 0xc9, 0x61, 0xf0, 0x22, 0x60,
0xa9, 0x04, 0x24, 0x02, 0xd0, 0x26, 0xa9, 0x01, 0x85, 0x02, 0x60, 0xa9, 0x08, 0x24, 0x02, 0xd0,
0x1b, 0xa9, 0x02, 0x85, 0x02, 0x60, 0xa9, 0x01, 0x24, 0x02, 0xd0, 0x10, 0xa9, 0x04, 0x85, 0x02,
0x60, 0xa9, 0x02, 0x24, 0x02, 0xd0, 0x05, 0xa9, 0x08, 0x85, 0x02, 0x60, 0x60, 0x20, 0x94, 0x06,
0x20, 0xa8, 0x06, 0x60, 0xa5, 0x00, 0xc5, 0x10, 0xd0, 0x0d, 0xa5, 0x01, 0xc5, 0x11, 0xd0, 0x07,
0xe6, 0x03, 0xe6, 0x03, 0x20, 0x2a, 0x06, 0x60, 0xa2, 0x02, 0xb5, 0x10, 0xc5, 0x10, 0xd0, 0x06,
0xb5, 0x11, 0xc5, 0x11, 0xf0, 0x09, 0xe8, 0xe8, 0xe4, 0x03, 0xf0, 0x06, 0x4c, 0xaa, 0x06, 0x4c,
0x35, 0x07, 0x60, 0xa6, 0x03, 0xca, 0x8a, 0xb5, 0x10, 0x95, 0x12, 0xca, 0x10, 0xf9, 0xa5, 0x02,
0x4a, 0xb0, 0x09, 0x4a, 0xb0, 0x19, 0x4a, 0xb0, 0x1f, 0x4a, 0xb0, 0x2f, 0xa5, 0x10, 0x38, 0xe9,
0x20, 0x85, 0x10, 0x90, 0x01, 0x60, 0xc6, 0x11, 0xa9, 0x01, 0xc5, 0x11, 0xf0, 0x28, 0x60, 0xe6,
0x10, 0xa9, 0x1f, 0x24, 0x10, 0xf0, 0x1f, 0x60, 0xa5, 0x10, 0x18, 0x69, 0x20, 0x85, 0x10, 0xb0,
0x01, 0x60, 0xe6, 0x11, 0xa9, 0x06, 0xc5, 0x11, 0xf0, 0x0c, 0x60, 0xc6, 0x10, 0xa5, 0x10, 0x29,
0x1f, 0xc9, 0x1f, 0xf0, 0x01, 0x60, 0x4c, 0x35, 0x07, 0xa0, 0x00, 0xa5, 0xfe, 0x91, 0x00, 0x60,
0xa6, 0x03, 0xa9, 0x00, 0x81, 0x10, 0xa2, 0x00, 0xa9, 0x07, 0x81, 0x10, 0x60, 0xa2, 0x00, 0xea,
0xea, 0xca, 0xd0, 0xfb, 0x60 };

    memcpy(mem+0x0600, program, sizeof(program));

// 0600: 20 06 06 20 38 06 20 0d 06 20 2a 06 60 a9 02 85 
// 0610: 02 a9 04 85 03 a9 11 85 10 a9 10 85 12 a9 0f 85 
// 0620: 14 a9 04 85 11 85 13 85 15 60 a5 fe 85 00 a5 fe 
// 0630: 29 03 18 69 02 85 01 60 20 4d 06 20 8d 06 20 c3 
// 0640: 06 20 19 07 20 20 07 20 2d 07 4c 38 06 a5 ff c9 
// 0650: 77 f0 0d c9 64 f0 14 c9 73 f0 1b c9 61 f0 22 60 
// 0660: a9 04 24 02 d0 26 a9 01 85 02 60 a9 08 24 02 d0 
// 0670: 1b a9 02 85 02 60 a9 01 24 02 d0 10 a9 04 85 02 
// 0680: 60 a9 02 24 02 d0 05 a9 08 85 02 60 60 20 94 06 
// 0690: 20 a8 06 60 a5 00 c5 10 d0 0d a5 01 c5 11 d0 07 
// 06a0: e6 03 e6 03 20 2a 06 60 a2 02 b5 10 c5 10 d0 06 
// 06b0: b5 11 c5 11 f0 09 e8 e8 e4 03 f0 06 4c aa 06 4c 
// 06c0: 35 07 60 a6 03 ca 8a b5 10 95 12 ca 10 f9 a5 02 
// 06d0: 4a b0 09 4a b0 19 4a b0 1f 4a b0 2f a5 10 38 e9 
// 06e0: 20 85 10 90 01 60 c6 11 a9 01 c5 11 f0 28 60 e6 
// 06f0: 10 a9 1f 24 10 f0 1f 60 a5 10 18 69 20 85 10 b0 
// 0700: 01 60 e6 11 a9 06 c5 11 f0 0c 60 c6 10 a5 10 29 
// 0710: 1f c9 1f f0 01 60 4c 35 07 a0 00 a5 fe 91 00 60 
// 0720: a6 03 a9 00 81 10 a2 00 a9 01 81 10 60 a2 00 ea 
// 0730: ea ca d0 fb 60 



    

    mem[0xfffc] = 0x00;
    mem[0xfffd] = 0x06;

    // TODO generate video that matches the easy6502 specification
    // NOTE more programs at https://github.com/r00ster91/easy6502

/*
    printf("> ");
    stdio_flush();
    char buffer[80];
    
    size_t size = getLine(buffer, 80, true);
    printf("Read %d bytes: %s", size, buffer);

    uint16_t addr;
    int result = sscanf(buffer, "%d", &addr);

    printf("%d  %04x\n", result, addr);
    */

   /*
    char action[4];
    int addr;
    int n=1;
    
    while (1) {
        int count = 0;
        int a[100];
   // Perform a do-while loop
    do {
 
        // Take input at position count
        // and increment count
        scanf("%d", &a[count++]);
        printf("C: %d %d\n", count, getchar());


        // If '\n' (newline) has occurred
        // or the whole array is filled,
        // then exit the loop
 
        // Otherwise, continue
    } while (getchar() != '\n' && count < 100);
        printf("STOP\n");

        int result = scanf("%s %x %d", action, &addr, &n);

        //printf("%d\n", result);
        printf("%04x: ", addr);
        for (int i=0; i<n; i++) {
            printf("%02x ", mem[addr++]);
        }
        printf("\n");

        if (!strcmp(action, "r")) {
            // read6502(addr);
        }
    }
    */
    reset6502();
    // for (int i=0; i<20; i++) {
    //     step6502();
    // }


    while (1) {
        exec6502(10);
        int ch = getchar_timeout_us(1000);
        if (ch != PICO_ERROR_TIMEOUT) {
            mem[0xff] = ch;
        }
    }
}
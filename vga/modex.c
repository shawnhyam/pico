#include "vga_inc.h"

void modex_draw_scanline_fragment(scanvideo_scanline_buffer_t *buffer) {
    // figure out 1/32 of the color value
    uint line_num = scanvideo_scanline_number(buffer->scanline_id);
    uint frame_num = scanvideo_frame_number(buffer->scanline_id);

    // uint32_t *dst32 = buffer->data;
    // *dst32++ = COMPOSABLE_COLOR_RUN | 0xffff;
    // *dst32++ = 637 | COMPOSABLE_RAW_1P_SKIP_ALIGN;
    // *dst32++ = 0 | COMPOSABLE_EOL_ALIGN;

    {
        uint16_t *p = (uint16_t *)buffer->data;
        if (line_num == 0 || line_num == 479) {
            *p++ = COMPOSABLE_COLOR_RUN;
            *p++ = PICO_SCANVIDEO_PIXEL_FROM_RGB5(31, 0, 0);
            *p++ = 315;
            *p++ = COMPOSABLE_RAW_1P;
            *p++ = 0;
            *p++ = COMPOSABLE_EOL_ALIGN;
            // *p++ = 0;

        } else {
            *p++ = COMPOSABLE_RAW_1P;
            *p++ = PICO_SCANVIDEO_PIXEL_FROM_RGB5(31, 0, 0);
            *p++ = COMPOSABLE_COLOR_RUN;
            *p++ = PICO_SCANVIDEO_PIXEL_FROM_RGB5(3, 3, 3);
            *p++ = 315;
            *p++ = COMPOSABLE_RAW_2P;
            *p++ = PICO_SCANVIDEO_PIXEL_FROM_RGB5(31, 0, 0);
            *p++ = 0;
            *p++ = COMPOSABLE_EOL_SKIP_ALIGN;
            *p++ = 0xffff;
        }

        buffer->data_used = ((uint32_t *)p) - buffer->data;
    }

    {
        uint16_t *p = (uint16_t *)buffer->data2;
        if (line_num == 16 || line_num == 463) {
            *p++ = COMPOSABLE_COLOR_RUN;
            *p++ = PICO_SCANVIDEO_ALPHA_MASK | PICO_SCANVIDEO_PIXEL_FROM_RGB5(0, 31, 0);
            *p++ = 637;
            *p++ = COMPOSABLE_RAW_1P;
            *p++ = 0;
            *p++ = COMPOSABLE_EOL_ALIGN;
            // *p++ = 0;

        } else {
            *p++ = COMPOSABLE_RAW_1P;
            *p++ = PICO_SCANVIDEO_ALPHA_MASK | PICO_SCANVIDEO_PIXEL_FROM_RGB8(255, 0, 0);
            *p++ = COMPOSABLE_COLOR_RUN;
            *p++ = PICO_SCANVIDEO_PIXEL_FROM_RGB8(0, 0, 0);
            *p++ = 635;
            *p++ = COMPOSABLE_RAW_2P;
            *p++ = PICO_SCANVIDEO_PIXEL_FROM_RGB8(255, 0, 0);
            *p++ = 0;
            *p++ = COMPOSABLE_EOL_SKIP_ALIGN;
            *p++ = 0xffff;
        }
        buffer->data2_used = ((uint32_t *)p) - buffer->data2;
    }

    {
        uint16_t *p = (uint16_t *)buffer->data3;
        if (line_num == 32 || line_num == 447) {
            *p++ = COMPOSABLE_COLOR_RUN;
            *p++ = PICO_SCANVIDEO_ALPHA_MASK | PICO_SCANVIDEO_PIXEL_FROM_RGB5(0, 0, 31);
            *p++ = 637;
            *p++ = COMPOSABLE_RAW_1P;
            *p++ = 0;
            *p++ = COMPOSABLE_EOL_ALIGN;
            // *p++ = 0;

        } else {
            *p++ = COMPOSABLE_RAW_1P;
            *p++ = PICO_SCANVIDEO_PIXEL_FROM_RGB8(255, 0, 0);
            *p++ = COMPOSABLE_COLOR_RUN;
            *p++ = PICO_SCANVIDEO_PIXEL_FROM_RGB8(0, 0, 0);
            *p++ = 635;
            *p++ = COMPOSABLE_RAW_2P;
            *p++ = PICO_SCANVIDEO_PIXEL_FROM_RGB8(255, 0, 0);
            *p++ = 0;
            *p++ = COMPOSABLE_EOL_SKIP_ALIGN;
            *p++ = 0xffff;
        }
        buffer->data3_used = ((uint32_t *)p) - buffer->data3;
    }

    buffer->status = SCANLINE_OK;
}

void modex_loop(void) {
    // initialize video and interrupts on core 1
    scanvideo_setup(&vga_mode);
    scanvideo_timing_enable(true);

    sem_release(&video_initted);

    // how can we support a configurable background color, behind layer 1?

    while (true) {
        scanvideo_scanline_buffer_t *scanline_buffer = scanvideo_begin_scanline_generation(true);
        modex_draw_scanline_fragment(scanline_buffer);
        scanvideo_end_scanline_generation(scanline_buffer);

        // can we trigger some work after the last scanline of a frame?
        // what is the best way to trigger work on core 1 during VSYNC? what
        // work would we perform? updating the frame data makes sense...
    }
}

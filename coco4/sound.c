#include <stdio.h>
#include <math.h>
#include "pico/audio_pwm.h"
#include "pico/stdlib.h"

// TODOs
// - try more than one channel
// - compare 8-bit vs 16-bit
// - allocate other wave tables (square, triangle, sawtooth, etc.)
// - investigate how to do ASDR envelope with this setup
// - experiment with other sample frequencies, although hopefully 24000 is good
// - do we have to update sample_stride when we change from 8-bit to 16-bit, plus wave_table of course
// - do we have the CPU horsepower to do stereo as well? would require 2 PIO's
// 
//
// Currently set up for 48MHz system clock, higher system clock == higher frequencies and vice versa;
// presumably, this means there's a lot of spare CPU power available to generate sound. Hopefully it's
// enough to do what I want.
//
// How to do ASDR: basically have to do it in software, there's nothing fancy going on. Right now
// it takes almost 0 time to generate the sound data, so hopefully adding an envelope will not be
// a big deal.




#define SINE_WAVE_TABLE_LEN 2048

// 512 means we can generate sound during VBLANK, this is 21.33ms of sound at 24kHz
// another option is to pretty much generate 1 sample at a time, during HBLANK?
#define SAMPLES_PER_BUFFER 512


static int16_t sine_wave_table[SINE_WAVE_TABLE_LEN];
static int16_t square_wave_table[SINE_WAVE_TABLE_LEN];

const audio_pwm_channel_config_t mono_channel_config =
        {
                .core = {
                        .base_pin = 28,
                        .pio_sm = 0,
                        .dma_channel = 0
                },
                .pattern = 3,
        };

struct audio_buffer_pool *init_audio() {
    printf("init_audio\n");

    static audio_format_t audio_format = {
        .format = AUDIO_BUFFER_FORMAT_PCM_S16,  // NOTE 8-bit is also available
        .sample_freq = 24000,
        .channel_count = 1,  // TODO try more channels
    };

    static struct audio_buffer_format producer_format = {
        .format = &audio_format,
        .sample_stride = 2
    };

    struct audio_buffer_pool *producer_pool = audio_new_producer_pool(&producer_format, 3,
                                                                      SAMPLES_PER_BUFFER); // todo correct size
    bool __unused ok;
    const struct audio_format *output_format;
    output_format = audio_pwm_setup(&audio_format, -1, &mono_channel_config);
    if (!output_format) {
        panic("PicoAudio: Unable to open audio device.\n");
    }
    ok = audio_pwm_default_connect(producer_pool, false);
    assert(ok);
    audio_pwm_set_enabled(true);
    return producer_pool;
}


int audio_test() {
    printf("audio_test\n");

    for (int i = 0; i < SINE_WAVE_TABLE_LEN; i++) {
        sine_wave_table[i] = 32767 * cosf(i * 2 * (float) (M_PI / SINE_WAVE_TABLE_LEN));
    }
    for (int i = 0; i < SINE_WAVE_TABLE_LEN; i++) {
        square_wave_table[i] = cosf(i * 2 * (float) (M_PI / SINE_WAVE_TABLE_LEN)) < 0 ? -32767 : 32767;
    }

    struct audio_buffer_pool *ap = init_audio();
    uint32_t step = 0x200000;
    uint32_t step2 = 0x200000;

    uint32_t pos = 0;
    uint32_t pos2 = 0;

    uint32_t pos_max = 0x10000 * SINE_WAVE_TABLE_LEN;
    uint vol = 32;
    while (true) {
        enum audio_correction_mode m = audio_pwm_get_correction_mode();
        int c = getchar_timeout_us(0);
        if (c >= 0) {
            if (c == '-' && vol) vol -= 4;
            if ((c == '=' || c == '+') && vol < 255) vol += 4;
            if (c == '[' && step > 0x10000) step -= 0x10000;
            if (c == ']' && step < (SINE_WAVE_TABLE_LEN / 16) * 0x20000) step += 0x10000;
            if (c == 'q') break;
            if (c == 'c') {
                bool done = false;
                while (!done) {
                    if (m == none) m = fixed_dither;
                    else if (m == fixed_dither) m = dither;
                    else if (m == dither) m = noise_shaped_dither;
                    else if (m == noise_shaped_dither) m = none;
                    done = audio_pwm_set_correction_mode(m);
                }
            }
            printf("vol = %d, step = %d mode = %d      \r", vol, step >>16, m);
        }

        struct audio_buffer *buffer = take_audio_buffer(ap, true);
        int16_t *samples = (int16_t *) buffer->buffer->bytes;
        for (uint i = 0; i < buffer->max_sample_count; i++) {
            samples[i] = (vol * sine_wave_table[pos >> 16u]) >> 9u;
            samples[i] += (vol * sine_wave_table[pos2 >> 16u]) >> 9u;

            pos += step;
            pos2 += step2;
            if (pos >= pos_max) pos -= pos_max;
            if (pos2 >= pos_max) pos2 -= pos_max;
        }
        buffer->sample_count = buffer->max_sample_count;
        give_audio_buffer(ap, buffer);
    }
    puts("\n");
    return 0;
}
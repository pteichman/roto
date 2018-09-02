/* Copyright (c) 2018 Peter Teichman */

#if defined(__cplusplus)
extern "C" {
#endif

#include "amfm.h"
#include "tonewheel_osc.h"

float remap_i16(int16_t v, int16_t oldmin, int16_t oldmax, int16_t newmin, int16_t newmax) {
    return newmin + (v - oldmin) * (newmax - newmin) / (oldmax - oldmin);
}

int16_t lerp_i16(int16_t a, int16_t b, uint16_t scale) {
    return ((0xFFFF - scale) * a + scale * b) >> 16;
}

// fill_sinemod fills a 256 element buffer with a sine wave. It starts
// from _phase_ (measured in 1<<15 ticks per cycle) and is mapped into
// the output range min..max (inclusive of both).
void fill_sinemod(int16_t ret[256], int16_t min, int16_t max, int32_t phase) {
    int32_t phase_incr = (1 << 15) / 256;
    for (int i = 0; i < 256; i++) {
        int16_t v = isin_S3(phase);
        phase += phase_incr;
        ret[i] = remap_i16(v, -(1 << 12), (1 << 12), min, max);
    }
}

void amfm_update(int16_t *dst, int16_t *src, int dstsrc_len, int16_t *ringbuf, int ringbuf_len, int *ringbuf_wp, int16_t *readVolume, int16_t *readOffset, uint32_t phaseIncr, uint32_t *phase_out) {
    int wp = *ringbuf_wp;
    uint32_t phase = *phase_out;

    // Read a block from the ring buffer, moving the read head
    // according to readOffset and modulating its output by
    // readVolume.
    for (int i = 0; i < dstsrc_len; i++) {
        // First, write the next sample to the ring buffer.
        ringbuf[wp] = src[i];

        // Figure out where the read pointer is. First: 8-bit
        // angle and interpolation scale.
        uint8_t index = phase >> 24;            // top 8 bits */
        uint16_t scale = (phase >> 8) & 0xFFFF; // next 16 bits */

        int16_t volume = lerp_i16(readVolume[index], readVolume[index + 1], scale);
        int16_t offset = readOffset[index];

        int rp = wp + offset;
        rp %= ringbuf_len;

        int16_t sample = ringbuf[rp];
        dst[i] = (sample * volume) >> 15;

        phase += phaseIncr;

        wp++;
        wp %= ringbuf_len;
    }

    *ringbuf_wp = wp;
    *phase_out = phase;
}

#if defined(__cplusplus)
}
#endif

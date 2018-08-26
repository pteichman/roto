/* Copyright (c) 2018 Peter Teichman */

#if defined(__cplusplus)
extern "C" {
#endif

#include "amfm.h"
#include "tonewheel_osc.h"

float remap_i16(int16_t v, int16_t oldmin, int16_t oldmax, int16_t newmin, int16_t newmax) {
    return newmin + (v - oldmin) * (newmax - newmin) / (oldmax - oldmin);
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

#if defined(__cplusplus)
}
#endif

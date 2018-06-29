/* Copyright (c) 2018 Peter Teichman */

#ifndef TONEWHEEL_OSC_H
#define TONEWHEEL_OSC_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

// tonewheel_osc simulates a set of Hammond B3 tonewheels.
typedef struct _tonewheel_osc {
    uint32_t phase_incrs[92];
    uint32_t phases[92];
    uint8_t volumes[92];
} tonewheel_osc;

tonewheel_osc *tonewheel_osc_new();
void tonewheel_osc_set_volume(tonewheel_osc *osc, uint8_t tonewheel, uint8_t volume);

void tonewheel_osc_fill(tonewheel_osc *osc, int16_t *block, size_t block_len);

#if defined(__cplusplus)
}
#endif

#endif

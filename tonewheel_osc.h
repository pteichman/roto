/* Copyright (c) 2018 Peter Teichman */

#ifndef _TONEWHEEL_OSC_H
#define _TONEWHEEL_OSC_H

extern "C" {

#include <stdint.h>

typedef struct _tonewheel_osc {
    uint32_t phase_incrs[92];
    uint32_t phases[92];
    uint8_t volumes[92];
} tonewheel_osc;

tonewheel_osc *tonewheel_osc_new();
void tonewheel_osc_set_volume(tonewheel_osc *osc, uint8_t tonewheel, uint8_t volume);

void tonewheel_osc_fill(tonewheel_osc *osc, int16_t *block, size_t block_len);
}

#endif

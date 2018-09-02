/* Copyright (c) 2018 Peter Teichman */

#ifndef AMFM_H
#define AMFM_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

// amfm is a combined amplitude and frequency modulation effect. It's
// intended to model the two speakers of a Leslie cabinet and
// eventually the Hammond vibrato circuit.
typedef struct _amfm {
    // gainmod and distmod are lookup tables for a gain multiplier and
    // a read offset (measured in samples) for the amfm effect.
    int16_t gainmod[256];
    int16_t distmod[256];
} amfm;

void amfm_set_gainmod(amfm *effect, uint16_t gainmod[256]);
void amfm_set_distmod(amfm *effect, uint16_t distmod[256]);

// fill_sinemod fills ret with a single cycle of a sine waveform
// starting from phase angle _phase_. The phase is measures as
// 2^15 units/circle.
void fill_sinemod(int16_t ret[256], int16_t min, int16_t max, int32_t phase);

void amfm_update(int16_t *dst, int16_t *src, int dstsrc_len, int16_t *ringbuf, int ringbuf_len, int *ringbuf_wp, int16_t *readVolume, int16_t *readOffset, uint32_t phaseIncr, uint32_t *phase_out);

#if defined(__cplusplus)
}
#endif

#endif

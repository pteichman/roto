/* Copyright (c) 2018 Peter Teichman */

#ifndef VIBRATO_AUDIO_H
#define VIBRATO_AUDIO_H

#include <Audio.h>

#include "vibrato.h"

// Simulate the Hammond Vibrato/Chorus scanner. Originally, this is a
// 1ms delay line with 9 taps. The circuit crossfades between the taps
// with a triangle wave from 1 to 9 and then back. This modulation wave
// has a frequency of 7Hz.

// A Teensy audio block is 128 samples at 44.1kHz, so roughly 2.9ms.
// We'll keep one block around to maintain a 1ms ring buffer. Each
// update() cycle reads the input block, writes it to the ring buffer,
// and writes phase modulated output.

enum VibratoMode {
    Off = 0,
    V1,
    V2,
    V3,
    C1,
    C2,
    C3,
};

// Vibrato implements the Vibrato/Chorus scanner of a Hammond B-3.
class Vibrato : public AudioStream {
  public:
    Vibrato() : AudioStream(1, inputQueueArray) {
    }

    void init() {
        // The ring buffer write pointer is initialized to give a
        // delay of 1ms.
        wp = 34; // 44117.64706Hz * 0.001s / 128 samples = 34 samples
        for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
            buf[i] = 0;
        }

        scan_phase = 0;
        setMode(Off);
    }

    void setMode(VibratoMode mode) {
        if (mode == V1) {
            depth = 3;
            mix = 0;
        } else if (mode == V2) {
            depth = 2;
            mix = 0;
        } else if (mode == V3) {
            depth = 1;
            mix = 0;
        } else if (mode == C1) {
            depth = 3;
            mix = 1;
        } else if (mode == C2) {
            depth = 2;
            mix = 1;
        } else if (mode == C3) {
            depth = 1;
            mix = 1;
        } else {
            depth = 8;
        }
    }

    void update(void) {
        audio_block_t *in = receiveReadOnly(0);
        if (in == NULL) {
            return;
        }

        audio_block_t *out = allocate();
        if (out == NULL) {
            release(in);
            return;
        }

        uint8_t loc_wp = wp;
        uint32_t loc_phase = scan_phase;

        for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
            // Write the input audio to our delay line.
            buf[loc_wp] = in->data[i];

            // Read the delay line into the output buffer.

            // 7 bits of loc_wp to 1..8; subtract triangle. This gives 5
            // bits of sway on a 7 bit counter.
            int32_t loc_rp = (loc_wp << 24) - (int32_t)(triangle(loc_phase) >> depth);

            int16_t pos = loc_rp >> 24;
            int16_t a = buf[pos & 0x7f];
            int16_t b = buf[++pos & 0x7f];

            int16_t val = lerp(a, b, (loc_rp >> 8) & 0xFFFF);
            if (mix) {
                val = ((int32_t)val + buf[loc_wp]) >> 1;
            }

            out->data[i] = val;

            loc_phase += 679632;

            // Increment and wrap loc_wp.
            loc_wp++;
            loc_wp &= 0x7f;
        }

        wp = loc_wp;
        scan_phase = loc_phase;

        transmit(out, 0);
        release(out);
        release(in);
    }

  private:
    uint32_t triangle(uint32_t phase) {
        // Convert 32-bit phase into signed 31-bit triangle.
        if (phase & 0x80000000) {
            return 0x80000000 + (0x80000000 - phase);
        } else {
            return phase;
        }
    }

    int32_t lerp(int32_t a, int32_t b, uint16_t scale) {
        return ((0xFFFF - scale) * a + scale * b) >> 16;
    }

    audio_block_t *inputQueueArray[1];

    // Ring buffer. The write pointer is stored here; the read pointer
    // is calculated at read time from wp.
    int wp;
    int16_t buf[AUDIO_BLOCK_SAMPLES];

    // Scanner position and scan direction. This modulates the read
    // pointer into buf as a 7Hz triangle wave. We'll use 15 bits for
    // position and the 16th to detect overflow.
    //
    // The write pointer is 34 samples ahead, so use 6 bits (5 + sign)
    // as the position modifier so reads never outrun writes.
    uint32_t scan_phase;

    // Depth of triangle scanner; 1..7, higher is *less* vibrato.
    int depth;
    int mix;
};

#endif

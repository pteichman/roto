/* Copyright (c) 2018 Peter Teichman */

#ifndef PREAMP_AUDIO_H
#define PREAMP_AUDIO_H

#include <Audio.h>
#include <math.h>

// Simulate the Leslie preamp. I got this from here:
// http://www.willpirkle.com/Downloads/Rotary%20Speaker%20Sim%20App%20Note.pdf
//
// There's a link to a book for the equation, but I haven't read it.

// Preamp implements the preamp distortion of a Leslie speaker cabinet.
class Preamp : public AudioStream {
  public:
    Preamp() : AudioStream(1, inputQueueArray) {
    }

    void setK(float k) {
        if (k <= 0) {
            k = 0.0001;
        }

        // y(n) = atan(k*x(n)) / atan(k)
        float invAtank = 1.0 / atan(k) * (float)(1 << 15);
        for (int i = 0; i < 65536; i++) {
            float kscale = k / (float)(1 << 15);
            lookup[i] = atan((float)(i - 32768) * kscale) * invAtank;
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

        int16_t *src = in->data;
        int16_t *dst = out->data;
        for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
            int32_t index = (int32_t)(src[i]) + 32768;

            if (index < 0) {
                index = 0;
            } else if (index > 65535) {
                index = 65535;
            }
            dst[i] = lookup[index];
        }

        transmit(out, 0);
        release(out);
        release(in);
    }

  private:
    int16_t lookup[65536];

    audio_block_t *inputQueueArray[1];
};

#endif

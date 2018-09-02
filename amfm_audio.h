/* Copyright (c) 2018 Peter Teichman */

#ifndef AMFM_AUDIO_H
#define AMFM_AUDIO_H

#include <Audio.h>

#include "amfm.h"

#define AMFM_RINGBUF_LEN (512)

// AmFm is a combined amplitude and frequency modulation effect. The
// amplitude and frequency offsets are both provided as lookup tables,
// indexed by the phase of a rotating angle. The phases are locked
// together, so this one effect can model the doppler and volume shift
// of a single rotating speaker.
class AmFm : public AudioStream {
  public:
    AmFm() : AudioStream(1, inputQueueArray) {
    }

    void init() {
        phase = 0;
        setDelayDepth(0);
        setTremoloDepth(0);
        setRotationRate(0);
        memset(ringbuf, 0, AMFM_RINGBUF_LEN * sizeof(int16_t));
    }

    // setDelayDepth sets the depth of the vibrato effect. This is
    // provided in milliseconds. The Leslie treble horn rotates
    // through 0.04064m of delay (the horn length is 8"). Assuming the
    // speed of sound is 344 m/s (sea level) this means a Leslie
    // induces 1.18ms of delay at its maximum.
    //
    // This effect incorporates a fixed length 512 sample ring
    // buffer, so at 44.1kHz the maximum delay available is
    // 512 * (1/44100) = 11.6ms.
    void setDelayDepth(float ms) {
        // Our readOffset starts from 0 (no delay) and increases from
        // there, so it's always subtracted from the ring buffer write
        // index.
        int16_t maxDelay = (int16_t)(44.1 * ms);

        // Clamp the delay to our ringbuffer length.
        if (maxDelay < 0) {
            maxDelay = 0;
        } else if (maxDelay > AMFM_RINGBUF_LEN) {
            maxDelay = AMFM_RINGBUF_LEN;
        }

        fill_sinemod(readOffset, 0, maxDelay, 0);
        readOffset[256] = readOffset[0];
    }

    // setTremoloDepth sets the depth of the tremolo effect. This is a
    // float provided in the range 0..1 (inclusive). 0 means no
    // effect, 1 means the signal is attenuated all the way to 0 once
    // per cycle.
    void setTremoloDepth(float depth) {
        int16_t maxVolume = 32767;
        int16_t minVolume = (uint16_t)((float)maxVolume * (1.0 - depth));

        if (minVolume < 0) {
            minVolume = 0;
        } else if (minVolume > maxVolume) {
            minVolume = maxVolume;
        }

        fill_sinemod(readVolume, minVolume, maxVolume, 0);
        readVolume[256] = readVolume[0];
    }

    // setRotationRate sets the rate of rotation of the effect (in
    // cycles per second).
    void setRotationRate(float hz) {
        // 1<<32 / 44100 = 97391.55
        phaseIncr = (uint32_t)(hz * 97391.55 + 0.5);
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

        // Making this overly complex in order to extract the logic
        // into amfm.cpp for offline testing. To be cleaned up later.
        amfm_update(out->data, in->data, AUDIO_BLOCK_SAMPLES, ringbuf, AMFM_RINGBUF_LEN, &wp, readVolume, readOffset, phaseIncr, &phase);

        transmit(out, 0);
        release(out);
        release(in);
    }

  private:
    // Ring buffer & its write position.
    int16_t ringbuf[AMFM_RINGBUF_LEN];
    int wp;

    // Phase increment & current angle for speaker rotation.
    uint32_t phaseIncr;
    uint32_t phase;

    // Modulation amounts for the read head & volume. This is a 256
    // value array with the first item duplicated at the end, so we
    // can index blindly off the end.
    int16_t readOffset[257];
    int16_t readVolume[257];

    audio_block_t *inputQueueArray[1];
};

#endif

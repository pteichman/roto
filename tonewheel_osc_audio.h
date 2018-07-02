/* Copyright (c) 2018 Peter Teichman */

#ifndef TONEWHEEL_OSC_AUDIO_H
#define TONEWHEEL_OSC_AUDIO_H

#include <Audio.h>
#include "tonewheel_osc.h"

// TonewheelOsc is a Teensy AudioStream wrapper around the
// tonewheel_osc oscillator block.
class TonewheelOsc : public AudioStream {
  public:
    TonewheelOsc() : AudioStream(0, NULL) {
    }

    void init() {
        osc = tonewheel_osc_new();
    }
    void update() {
        audio_block_t *block;
        block = allocate();
        if (!block) {
            return;
        }
        tonewheel_osc_fill(osc, block->data, AUDIO_BLOCK_SAMPLES);
        transmit(block, 0);
        release(block);
    }

    void setVolume(uint8_t tonewheel, uint8_t volume) {
        tonewheel_osc_set_volume(osc, tonewheel, volume);
    }

    void setVolumes(uint16_t volumes[92]) {
        for (int i = 1; i < 92; i++) {
            tonewheel_osc_set_volume(osc, i, volumes[i] >> 6);
        }
    }

  private:
    tonewheel_osc *osc;
};

#endif

/* Copyright (c) 2018 Peter Teichman */

#ifndef MONITOR_AUDIO_H
#define MONITOR_AUDIO_H

#include <Audio.h>

// Monitor is an audio device that monitors the output levels of an
// audio channel. It's useful for seeing how much headroom is
// available in the 16 bit space.
class Monitor : public AudioStream {
  public:
    Monitor() : AudioStream(1, inputQueueArray) {
    }

    void init() {
	min_vol = 0;
	max_vol = 0;
	min_vol_ever = 0;
	max_vol_ever = 0;
    }

    void update() {
        audio_block_t *in = receiveReadOnly(0);
        if (in == NULL) {
            return;
        }

	for (int i=0; i<AUDIO_BLOCK_SAMPLES; i++) {
	    int16_t v = in->data[i];
	    if (v < min_vol) {
		min_vol = v;
		if (v < min_vol_ever) {
		    min_vol_ever = v;
		}
	    }
	    if (v > max_vol) {
		max_vol = v;
		if (v > max_vol_ever) {
		    max_vol_ever = v;
		}
	    }
	}

        transmit(in, 0);
        release(in);
    }

    void reset() {
	min_vol = 0;
	max_vol = 0;
    }

    int volumeUsageMin() {
	return min_vol;
    }

    int volumeUsageMax() {
	return max_vol;
    }

    int volumeUsageMinEver() {
	return max_vol_ever;
    }

    int volumeUsageMaxEver() {
	return max_vol_ever;
    }

  private:
    int min_vol;
    int max_vol;
    int min_vol_ever;
    int max_vol_ever;

    audio_block_t *inputQueueArray[1];
};

#endif

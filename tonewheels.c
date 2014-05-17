/* Copyright (c) 2011 Peter Teichman */

#include "tonewheels.h"

/* 91 tonewheels, plus one so arrays can be 1-indexed and use
   real-life tonewheel numbering */
#define NUM_TONEWHEELS (92)
#define NUM_DRAWBARS (9)

/* 61 keys, plus one for 1-indexing */
#define NUM_KEYS (62)

#define MAX_TONEWHEEL_VOLUME (16)

static uint16_t tonewheel_positions[NUM_TONEWHEELS];
static uint8_t tonewheel_volumes[NUM_TONEWHEELS];

static uint8_t num_active_tonewheels;
static uint8_t active_tonewheels[NUM_TONEWHEELS];

static uint8_t active_keys[NUM_KEYS];

/* Calculated from http://www.goodeveca.net/RotorOrgan/ToneWheelSpec.html,
 * given a 15625Hz timer. Each rate is (2^16 * freq) / 15625. */
static const uint16_t tonewheel_rates[] = {
    0,   /* 1-indexed to allow us to use real-life tonewheel numbering */
    178, /* C0, MIDI note 12 */
    188, /* C#0 */
    200, /* D0 */
    212, /* D#0 */
    224, /* E0 */
    237, /* F0 */
    252, /* F#0 */
    267, /* G0 */
    282, /* G#0 */
    299, /* A0 */
    317, /* A#0 */
    336, /* B0 */
    356, /* C1 */
    377, /* C#1 */
    400, /* D1 */
    424, /* D#1 */
    449, /* E1 */
    475, /* F1 */
    504, /* F#1 */
    534, /* G1 */
    565, /* G#1 */
    599, /* A1 */
    635, /* A#1 */
    672, /* B1 */
    712, /* C2 */
    755, /* C#2 */
    800, /* D2 */
    848, /* D#2 */
    898, /* E2 */
    951, /* F2 */
    1008, /* F#2 */
    1068, /* G2 */
    1131, /* G#2 */
    1199, /* A2 */
    1270, /* A#2 */
    1345, /* B2 */
    1425, /* C3 */
    1510, /* C#3 */
    1601, /* D3 */
    1696, /* D#3 */
    1796, /* E3 */
    1903, /* F3 */
    2017, /* F#3 */
    2137, /* G3 */
    2263, /* G#3 */
    2398, /* A3 */
    2541, /* A#3 */
    2691, /* B3 */
    2851, /* C4 */
    3021, /* C#4 */
    3202, /* D4 */
    3392, /* D#4 */
    3593, /* E4 */
    3806, /* F4 */
    4034, /* F#4 */
    4274, /* G4 */
    4526, /* G#4 */
    4797, /* A4 */
    5082, /* A#4 */
    5383, /* B4 */
    5703, /* C5 */
    6042, /* C#5 */
    6404, /* D5 */
    6784, /* D#5 */
    7187, /* E5 */
    7612, /* F5 */
    8068, /* F#5 */
    8548, /* G5 */
    9052, /* G#5 */
    9595, /* A5 */
    10164, /* A#5 */
    10766, /* B5 */
    11406, /* C6 */
    12084, /* C#6 */
    12809, /* D6 */
    13568, /* D#6 */
    14375, /* E6 */
    15225, /* F6 */
    16137, /* F#6 */
    17096, /* G6 */
    18105, /* G#6 */
    19190, /* A6 */
    20328, /* A#6 */
    21533, /* B6 */
    22838, /* ~C7 */
    24205, /* ~C#7 */
    25645, /* ~D7 */
    27158, /* ~D#7 */
    28785, /* ~E7 */
    30492, /* ~F7 */
    32299  /* ~F#7 */
};

static const int8_t sine[] = {
     0x00,  0x03,  0x06,  0x09,  0x0c,  0x10,  0x13,  0x16,  0x19,  0x1c,
     0x1f,  0x22,  0x25,  0x28,  0x2b,  0x2e,  0x31,  0x33,  0x36,  0x39,
     0x3c,  0x3f,  0x41,  0x44,  0x47,  0x49,  0x4c,  0x4e,  0x51,  0x53,
     0x55,  0x58,  0x5a,  0x5c,  0x5e,  0x60,  0x62,  0x64,  0x66,  0x68,
     0x6a,  0x6b,  0x6d,  0x6f,  0x70,  0x71,  0x73,  0x74,  0x75,  0x76,
     0x78,  0x79,  0x7a,  0x7a,  0x7b,  0x7c,  0x7d,  0x7d,  0x7e,  0x7e,
     0x7e,  0x7f,  0x7f,  0x7f,  0x7f,  0x7f,  0x7f,  0x7f,  0x7e,  0x7e,
     0x7e,  0x7d,  0x7d,  0x7c,  0x7b,  0x7a,  0x7a,  0x79,  0x78,  0x76,
     0x75,  0x74,  0x73,  0x71,  0x70,  0x6f,  0x6d,  0x6b,  0x6a,  0x68,
     0x66,  0x64,  0x62,  0x60,  0x5e,  0x5c,  0x5a,  0x58,  0x55,  0x53,
     0x51,  0x4e,  0x4c,  0x49,  0x47,  0x44,  0x41,  0x3f,  0x3c,  0x39,
     0x36,  0x33,  0x31,  0x2e,  0x2b,  0x28,  0x25,  0x22,  0x1f,  0x1c,
     0x19,  0x16,  0x13,  0x10,  0x0c,  0x09,  0x06,  0x03,  0x00, -0x03,
    -0x06, -0x09, -0x0c, -0x10, -0x13, -0x16, -0x19, -0x1c, -0x1f, -0x22,
    -0x25, -0x28, -0x2b, -0x2e, -0x31, -0x33, -0x36, -0x39, -0x3c, -0x3f,
    -0x41, -0x44, -0x47, -0x49, -0x4c, -0x4e, -0x51, -0x53, -0x55, -0x58,
    -0x5a, -0x5c, -0x5e, -0x60, -0x62, -0x64, -0x66, -0x68, -0x6a, -0x6b,
    -0x6d, -0x6f, -0x70, -0x71, -0x73, -0x74, -0x75, -0x76, -0x78, -0x79,
    -0x7a, -0x7a, -0x7b, -0x7c, -0x7d, -0x7d, -0x7e, -0x7e, -0x7e, -0x7f,
    -0x7f, -0x7f, -0x7f, -0x7f, -0x7f, -0x7f, -0x7e, -0x7e, -0x7e, -0x7d,
    -0x7d, -0x7c, -0x7b, -0x7a, -0x7a, -0x79, -0x78, -0x76, -0x75, -0x74,
    -0x73, -0x71, -0x70, -0x6f, -0x6d, -0x6b, -0x6a, -0x68, -0x66, -0x64,
    -0x62, -0x60, -0x5e, -0x5c, -0x5a, -0x58, -0x55, -0x53, -0x51, -0x4e,
    -0x4c, -0x49, -0x47, -0x44, -0x41, -0x3f, -0x3c, -0x39, -0x36, -0x33,
    -0x31, -0x2e, -0x2b, -0x28, -0x25, -0x22, -0x1f, -0x1c, -0x19, -0x16,
    -0x13, -0x10, -0x0c, -0x09, -0x06, -0x03
};

/* Current drawbar position (0..8) for each of the 9 drawbars. */
static uint8_t drawbar_position[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* Current gain contributed by each drawbar (Q4.3 format) */
static uint8_t drawbar_gain[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* Map drawbar position (0..8) to the gain specified by that
 * position. This is in Q4.3 format, in 3dB increments.
 *
 * position | gain  | Q4.3 gain
 * 0        | 0     | 0
 * 1        | 1.414 | 11
 * 2        | 2     | 16
 * 3        | 2.828 | 23
 * 4        | 4     | 32
 * 5        | 5.657 | 45
 * 6        | 8     | 64
 * 7        | 11.31 | 90
 * 8        | 16    | 127
 */
static uint8_t drawbar_position_to_gain[] = { 0, 11, 16, 23, 32, 45, 64, 90, 127 };

/* Drawbar harmonics, used in get_drawbar_tonewheel()
 *
 * 16' (lower fundamental)
 * 5 1/3' (lower 3rd fundamental)
 * 8' (fundamental),
 * 4' (first even harmonic),
 * 2 2/3' (first odd),
 * 2' (second even),
 * 1 3/5' (second odd),
 * 1 1/7' (third odd),
 * 1' (third even) */
static const uint8_t drawbar_harmonics[] =
    { 0, 19, 12, 24, 31, 36, 40, 43, 48 };

void tonewheels_init() {
    uint8_t i;
    for(i=0; i<NUM_TONEWHEELS; i++) {
        tonewheel_volumes[i] = 0;
    }
}

void tonewheels_sample_v(int16_t *samples, uint8_t len) {
    uint16_t pos, rate;
    uint8_t wheel, vol;
    uint8_t i;
    uint8_t *active = active_tonewheels;
    int8_t sample;

    /* clear the samples */
    for (i=0; i<len; i++) {
        samples[i] = 0;
    }

    while ((wheel = *(active++))) {
        pos = tonewheel_positions[wheel];
        vol = tonewheel_volumes[wheel] >> 4; /* convert from Q4.4 */
        rate = tonewheel_rates[wheel];

        for (i=0; i<len; i++) {
            pos += rate;

            /* mimic the complex tone of the lower tonewheels */
            if (wheel < 13) {
                sample = (pos & 0b10000000) ? 0xFF : 0x00;
            } else {
                sample = sine[pos >> 8];
            }

            samples[i] += sample * vol;
        }

        tonewheel_positions[wheel] = pos;
    }
}

/* Tonewheel -> drawbar mappings from the table in
 * http://theatreorgans.com/hammond/faq/a-100/a-100.html. Done in code
 * to save space. */
uint8_t get_drawbar_tonewheel(uint8_t key, uint8_t drawbar) {
    uint8_t tonewheel = key + drawbar_harmonics[drawbar];

    /* adjustments to introduce upper/lower foldback */
    if (tonewheel < 13) {
        tonewheel = tonewheel  + 12;
    } else if (tonewheel > 103) {
        tonewheel = tonewheel - 24;
    } else if (tonewheel > 91) {
        tonewheel = tonewheel - 12;
    }

    return tonewheel;
}

static void tonewheels_rescan_active() {
    /* scan the active tonewheels (tonewheel_volumes[wheel] != 0) and
       pack them into active_tonewheels for quicker iteration in
       tonewheels_sample_v() */

    uint8_t i, j=0;

    for (i=0; i<NUM_TONEWHEELS; i++) {
        if (tonewheel_volumes[i] != 0) {
            active_tonewheels[j++] = i;
        }
    }

    num_active_tonewheels = j;
    active_tonewheels[j] = 0;
}

static void tonewheels_add_key_drawbars(uint8_t key) {
    uint8_t i;
    uint8_t tonewheel;
    uint8_t prev_volume;
    uint8_t volume;

    for (i=0; i<NUM_DRAWBARS; i++) {
        tonewheel = get_drawbar_tonewheel(key, i);
        prev_volume = tonewheel_volumes[tonewheel];
        volume = prev_volume + drawbar_gain[i];

        /* Check for overflow, constrain to 127. Since volume is in Q4.3
         * format, this leaves us with a gain of 15.9 (about 16). */
        if (volume < prev_volume) {
            volume = 127;
        }

        tonewheel_volumes[tonewheel] = volume;
    }
}

void tonewheels_scan_active_keys() {
    /* apply volumes to tonewheels based on the keys that are down */
    uint8_t tonewheel;
    uint8_t volume;
    uint8_t i;
    uint8_t j;

    tonewheels_init();

    for (i=0; i<NUM_KEYS; i++) {
        if (!active_keys[i])
            continue;

        tonewheels_add_key_drawbars(i);
    }
}

void tonewheels_key_down(uint8_t key) {
    if (key == 0 || key > NUM_KEYS) {
        return;
    }

    if (!active_keys[key]) {
        active_keys[key] = 1;
        tonewheels_scan_active_keys();
    }

    /* compact the list of active tonewheels */
    tonewheels_rescan_active();
}

void tonewheels_key_up(uint8_t key) {
    if (key == 0 || key > NUM_KEYS) {
        return;
    }

    if (active_keys[key]) {
        active_keys[key] = 0;
        tonewheels_scan_active_keys();
    }

    /* compact the list of active tonewheels */
    tonewheels_rescan_active();
}

void tonewheels_set_drawbar(uint8_t drawbar, uint8_t value) {
    uint8_t i;

    if (drawbar > NUM_DRAWBARS) {
        return;
    }

    if (value > 8) {
        return;
    }

    drawbar = drawbar - 1;

    drawbar_position[drawbar] = value;
    drawbar_gain[drawbar] = drawbar_position_to_gain[value];

    /* zero out the active tonewheels and recreate them from active keys */
    tonewheels_init();
    for (i=0; i<NUM_KEYS; i++) {
        if (active_keys[i]) {
            tonewheels_add_key_drawbars(i);
        }
    }

    /* compact the list of active tonewheels */
    tonewheels_rescan_active();
}

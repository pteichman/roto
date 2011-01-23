/* Copyright (c) 2011 Peter Teichman */

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
 * given a 15625Hz timer. Each rate is (15625 * freq) >> 12. */
static const uint16_t tonewheel_rates[] = {
    0,   /* 1-indexed to allow us to use real-life tonewheel numbering */
    124, /* C0, MIDI note 12 */
    132, /* C#0 */
    140, /* D0 */
    148, /* D#0 */
    157, /* E0 */
    166, /* F0 */
    176, /* F#0 */
    186, /* G0 */
    197, /* G#0 */
    209, /* A0 */
    222, /* A#0 */
    235, /* B0 */
    249, /* C1 */
    264, /* C#1 */
    280, /* D1 */
    296, /* D#1 */
    314, /* E1 */
    332, /* F1 */
    352, /* F#1 */
    373, /* G1 */
    395, /* G#1 */
    419, /* A1 */
    444, /* A#1 */
    470, /* B1 */
    498, /* C2 */
    528, /* C#2 */
    560, /* D2 */
    593, /* D#2 */
    628, /* E2 */
    665, /* F2 */
    705, /* F#2 */
    747, /* G2 */
    791, /* G#2 */
    839, /* A2 */
    888, /* A#2 */
    941, /* B2 */
    997, /* C3 */
    1056, /* C#3 */
    1120, /* D3 */
    1186, /* D#3 */
    1257, /* E3 */
    1331, /* F3 */
    1411, /* F#3 */
    1495, /* G3 */
    1583, /* G#3 */
    1678, /* A3 */
    1777, /* A#3 */
    1883, /* B3 */
    1995, /* C4 */
    2113, /* C#4 */
    2240, /* D4 */
    2373, /* D#4 */
    2514, /* E4 */
    2663, /* F4 */
    2822, /* F#4 */
    2990, /* G4 */
    3167, /* G#4 */
    3356, /* A4 */
    3555, /* A#4 */
    3766, /* B4 */
    3990, /* C5 */
    4227, /* C#5 */
    4481, /* D5 */
    4747, /* D#5 */
    5029, /* E5 */
    5326, /* F5 */
    5645, /* F#5 */
    5981, /* G5 */
    6334, /* G#5 */
    6713, /* A5 */
    7111, /* A#5 */
    7533, /* B5 */
    7981, /* C6 */
    8455, /* C#6 */
    8962, /* D6 */
    9494, /* D#6 */
    10058, /* E6 */
    10653, /* F6 */
    11291, /* F#6 */
    11962, /* G6 */
    12668, /* G#6 */
    13427, /* A6 */
    14223, /* A#6 */
    15066, /* B6 */
    15980, /* ~C7 */
    16937, /* ~C#7 */
    17944, /* ~D7 */
    19003, /* ~D#7 */
    20141, /* ~E7 */
    21335, /* ~F7 */
    22600 /* ~F#7 */
};

static const uint8_t sine[] = {
    0x80, 0x83, 0x86, 0x89, 0x8C, 0x8F, 0x92, 0x95, 0x98, 0x9B, 0x9E, 0xA2,
    0xA5, 0xA7, 0xAA, 0xAD, 0xB0, 0xB3, 0xB6, 0xB9, 0xBC, 0xBE, 0xC1, 0xC4,
    0xC6, 0xC9, 0xCB, 0xCE, 0xD0, 0xD3, 0xD5, 0xD7, 0xDA, 0xDC, 0xDE, 0xE0,
    0xE2, 0xE4, 0xE6, 0xE8, 0xEA, 0xEB, 0xED, 0xEE, 0xF0, 0xF1, 0xF3, 0xF4,
    0xF5, 0xF6, 0xF8, 0xF9, 0xFA, 0xFA, 0xFB, 0xFC, 0xFD, 0xFD, 0xFE, 0xFE,
    0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFE, 0xFE, 0xFD,
    0xFD, 0xFC, 0xFB, 0xFA, 0xFA, 0xF9, 0xF8, 0xF6, 0xF5, 0xF4, 0xF3, 0xF1,
    0xF0, 0xEE, 0xED, 0xEB, 0xEA, 0xE8, 0xE6, 0xE4, 0xE2, 0xE0, 0xDE, 0xDC,
    0xDA, 0xD7, 0xD5, 0xD3, 0xD0, 0xCE, 0xCB, 0xC9, 0xC6, 0xC4, 0xC1, 0xBE,
    0xBC, 0xB9, 0xB6, 0xB3, 0xB0, 0xAD, 0xAA, 0xA7, 0xA5, 0xA2, 0x9E, 0x9B,
    0x98, 0x95, 0x92, 0x8F, 0x8C, 0x89, 0x86, 0x83, 0x80, 0x7D, 0x7A, 0x77,
    0x74, 0x71, 0x6E, 0x6B, 0x68, 0x65, 0x62, 0x5E, 0x5B, 0x59, 0x56, 0x53,
    0x50, 0x4D, 0x4A, 0x47, 0x44, 0x42, 0x3F, 0x3C, 0x3A, 0x37, 0x35, 0x32,
    0x30, 0x2D, 0x2B, 0x29, 0x26, 0x24, 0x22, 0x20, 0x1E, 0x1C, 0x1A, 0x18,
    0x16, 0x15, 0x13, 0x12, 0x10, 0x0F, 0x0D, 0x0C, 0x0B, 0x0A, 0x08, 0x07,
    0x06, 0x06, 0x05, 0x04, 0x03, 0x03, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x03, 0x03, 0x04, 0x05, 0x06,
    0x06, 0x07, 0x08, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x12, 0x13, 0x15,
    0x16, 0x18, 0x1A, 0x1C, 0x1E, 0x20, 0x22, 0x24, 0x26, 0x29, 0x2B, 0x2D,
    0x30, 0x32, 0x35, 0x37, 0x3A, 0x3C, 0x3F, 0x42, 0x44, 0x47, 0x4A, 0x4D,
    0x50, 0x53, 0x56, 0x59, 0x5B, 0x5E, 0x62, 0x65, 0x68, 0x6B, 0x6E, 0x71,
    0x74, 0x77, 0x7A, 0x7D
};

/* Current drawbar position (0..8) for each of the 9 drawbars. */
static uint8_t drawbar_position[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* Current gain contributed by each drawbar (Q4.4 format) */
static uint8_t drawbar_gain[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* Map drawbar position (0..8) to the gain specified by that
 * position. This is in Q4.4 format, in 3dB increments.
 *
 * position | gain  | Q4.4 gain
 * 0        | 0     | 0
 * 1        | 1.414 | 23
 * 2        | 2     | 32
 * 3        | 2.828 | 45
 * 4        | 4     | 64
 * 5        | 5.657 | 91
 * 6        | 8     | 128
 * 7        | 11.31 | 181
 * 8        | 16    | 255
 */
static uint8_t drawbar_position_to_gain[] = { 0, 23, 32, 45, 64, 91, 128, 181, 255 };

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

void tonewheels_sample_v(uint16_t *samples, uint8_t len) {
    uint16_t pos, rate;
    uint8_t wheel, vol;
    uint8_t i;
    uint8_t *active = active_tonewheels;

    while ((wheel = *(active++))) {
        pos = tonewheel_positions[wheel];
        vol = tonewheel_volumes[wheel] >> 4; /* convert from Q4.4 */
        rate = tonewheel_rates[wheel];

        for (i=0; i<len; i++) {
            pos += rate;
            samples[i] += sine[pos >> 8] * vol;
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

        /* Check for overflow, constrain to 255. Since volume is in Q4.4
         * format, this leaves us with a gain of 15.9 (about 16). */
        if (volume < prev_volume) {
            volume = 255;
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

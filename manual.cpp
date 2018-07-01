/* Copyright (c) 2018 Peter Teichman */

#if defined(__cplusplus)
extern "C" {
#endif

#include <string.h>

#include "manual.h"

// manual is here to maintain the mapping between physical keys on the
// manuals of an organ and the tonewheel oscillator. It takes key
// events and translates those to tonewheel volumes.
//
// It is aware of several physical properties of the Hammond B-3: the
// resistances of each wiring between keys and tonewheels, the
// resistances of each drawbar setting, and the factory recommended
// voltage from each tonewheel.

// foldback wraps calculated tonewheel values outside of the keyboard
// range (13..92) back into that range. The 1..13 tonewheels are
// controlled by the Hammond foot pedals, which is why they're not
// included here.
int foldback(uint8_t tonewheel) {
    while (tonewheel < 13) {
        tonewheel += 12;
    }
    while (tonewheel > 91) {
        tonewheel -= 12;
    }
    return tonewheel;
}

// tonewheel returns the number of the tonewheel connected to _key_ at
// _drawbar_.
int tonewheel(int key, int drawbar) {
    switch (drawbar) {
    case 1: // Sub-octave; 16'
        return foldback(key);
    case 2: // 5th; 5 1/3'
        return foldback(key + 19);
    case 3: // Unison; 8'
        return foldback(key + 12);
    case 4: // 8th (Octave); 4'
        return foldback(key + 24);
    case 5: // 12th; 2 2/3'
        return foldback(key + 31);
    case 6: // 15th; 2'
        return foldback(key + 36);
    case 7: // 17th; 1 3/5'
        return foldback(key + 40);
    case 8: // 19th; 1 1/3'
        return foldback(key + 43);
    case 9: // 22nd; 1'
        return foldback(key + 48);
    }
    return 0;
}

// manual_set_drawbars adjusts the drawbar settings on a manual.
// drawbars contains 9 integers in the 0..9 range.
void manual_set_drawbars(manual *m, uint8_t drawbars[9]) {
    memcpy(m->drawbars, drawbars, 9);
}

// manual_set_keys adjusts the keys currently pressed on a manual.
// keys contains 61 integers, nonzero if pressed.
void manual_set_keys(manual *m, uint8_t keys[61]) {
    memcpy(m->keys, keys, 61);
}

// manual_fill_volumes returns the current set of tonewheel volumes,
// measured in mA.
void manual_fill_volumes(manual *m, uint16_t volumes[92]) {
    // tonewheel t; key k; drawbar d.
    for (int t = 0; t < 92; t++) {
        for (int k = 0; k < 62; k++) {
        }
    }
}

#if defined(__cplusplus)
}
#endif

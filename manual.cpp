/* Copyright (c) 2018 Peter Teichman */

#if defined(__cplusplus)
extern "C" {
#endif

#include "manual.h"

// manual is here to maintain the mapping between physical keys on the
// manuals of an organ and the tonewheel oscillator. It takes key
// events and translates those to tonewheel volumes.
//
// It is aware of several physical properties of the Hammond B-3: the
// resistances of each wiring between keys and tonewheels, the
// resistances of each drawbar setting, and the factory recommended
// voltage from each tonewheel.

// resistance & friends return the resistance of the wire (in ohms)
// connected to the tonewheel for key + drawbar.
float resistance(int key, int drawbar);
float resistance1(int key);
float resistance2(int key);
float resistance3(int key);
float resistance4(int key);
float resistance5(int key);
float resistance6(int key);
float resistance7(int key);
float resistance8(int key);
float resistance9(int key);

// voltages taken from the "Post 1956 TG" table at HammondWiki. Using
// the Vpp levels, which feels right given that we're adding peak to
// peak volumes, but my EE memory is hazy here.
// http://www.dairiki.org/HammondWiki/ToneWheelGeneratorOutputLevels
float voltages[92] = {
    // Add an empty first element so voltages can be 1-indexed.
    0,
    70.0, 69.2, 68.3, 67.3, 66.4, 65.5, 64.5, 63.6, 62.6, 61.7, 60.8, 60.0,
    15.0, 14.6, 14.3, 14.0, 13.6, 13.3, 13.0, 12.6, 12.3, 12.0, 11.6, 11.3,
    11.0, 11.0, 11.0, 11.0, 11.0, 11.0, 11.0, 11.0, 11.0, 11.0, 11.0, 11.0,
    11.1, 11.2, 11.3, 11.4, 11.5, 11.7, 11.8, 12.0, 12.2, 12.5, 12.8, 13.0,
    13.2, 13.4, 13.6, 14.0, 14.2, 14.5, 14.7, 15.1, 15.2, 15.6, 15.8, 16.0,
    16.3, 16.6, 17.0, 17.3, 17.7, 18.0, 18.5, 18.8, 19.2, 19.4, 19.6, 19.8,
    20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0,
    20.0, 19.7, 19.3, 19.0, 18.7, 18.3, 18.0,
};

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

// drawbar_volume returns the volume multiplier for a drawbar set to
// _value_. It's scaled such that each drawbar stop doubles the power
// output, normalized to 0.0..1.0. Turns out this is value / 8.
float drawbar_volume(uint8_t value) {
    // (sqrt(2) * value) / (sqrt(2) * 8) = 0.125 * value
    return 0.125 * (float)value;
}

// resistance returns the resistance connected to _key_ at _drawbar_.
// This implements the resistance tapering table here:
// http://www.dairiki.org/HammondWiki/ManualTapering
float resistance(int key, int drawbar) {
    switch (drawbar) {
    case 1:
        return resistance1(key);
    case 2:
        return resistance2(key);
    case 3:
        return resistance3(key);
    case 4:
        return resistance4(key);
    case 5:
        return resistance5(key);
    case 6:
        return resistance6(key);
    case 7:
        return resistance7(key);
    case 8:
        return resistance8(key);
    case 9:
        return resistance9(key);
    }
    return 0;
}

float resistance1(int key) {
    if (key < 11) {
        return 100;
    } else if (key < 17) {
        return 50;
    } else if (key < 25) {
        return 34;
    } else if (key < 37) {
        return 24;
    } else if (key < 49) {
        return 15;
    } else {
        return 10;
    }
}

float resistance2(int key) {
    if (key < 15) {
        return 34;
    } else if (key < 39) {
        return 24;
    } else if (key < 15) {
        return 15;
    } else {
        return 10;
    }
}

float resistance3(int key) {
    if (key < 16) {
        return 50;
    } else if (key < 24) {
        return 34;
    } else if (key < 38) {
        return 24;
    } else if (key < 50) {
        return 15;
    } else {
        return 10;
    }
}

float resistance4(int key) {
    if (key < 14) {
        return 34;
    } else if (key < 40) {
        return 24;
    } else {
        return 34;
    }
}

float resistance5(int key) {
    if (key < 13) {
        return 10;
    } else if (key < 21) {
        return 15;
    } else if (key < 41) {
        return 24;
    } else if (key < 53) {
        return 34;
    } else {
        return 50;
    }
}

float resistance6(int key) {
    if (key < 12) {
        return 10;
    } else if (key < 21) {
        return 15;
    } else if (key < 42) {
        return 24;
    } else if (key < 56) {
        return 34;
    } else {
        return 50;
    }
}

float resistance7(int key) {
    if (key < 19) {
        return 10;
    } else if (key < 43) {
        return 24;
    } else if (key < 52) {
        return 34;
    } else {
        return 50;
    }
}

float resistance8(int key) {
    if (key < 44) {
        return 24;
    } else if (key < 49) {
        return 34;
    } else {
        return 50;
    }
}

float resistance9(int key) {
    if (key < 44) {
        return 24;
    } else {
        return 50;
    }
}

// manual_max_volume returns the sum of all volumes at maximum
// output. This is used to normalize each drawbar volume.
float manual_max_volume() {
    float invR[92] = {0};

    for (int k = 1; k < 62; k++) {
        for (int d = 1; d < 10; d++) {
            int t = tonewheel(k, d);
            invR[t] += 1.0 / resistance(k, d);
        }
    }

    float sum = 0.0;
    for (int t = 1; t < 92; t++) {
        sum += voltages[t] * invR[t];
    }
    return sum;
}

// manual_fill_volumes returns the current set of tonewheel volumes,
// with values in the Q14 range. keys is an array of 61 keys on a
// manual, zero indexed and nonzero if pressed. drawbars contains the
// resistance at each of the 9 drawbars, also zero indexed.
//
// drawbars[1]: 16' (sub-octave)
// drawbars[2]: 5 1/3' (5th)
// drawbars[3]: 8' (unison)
// drawbars[4]: 4' (8th)
// drawbars[5]: 2 2/3' (12th)
// drawbars[6]: 2' (15th)
// drawbars[7]: 1 3/5' (15th)
// drawbars[8]: 1 1/3' (19th)
// drawbars[9]: 1' (22nd)
void manual_fill_volumes(uint8_t keys[62], uint8_t drawbars[10], uint16_t ret[92]) {
    // invR is the inverse resistance connected from tonewheel t.
    float invR[92] = {0};

    for (int k = 1; k < 62; k++) {
        if (keys[k] == 0) {
            continue;
        }

        for (int d = 1; d < 10; d++) {
            if (drawbars[d] == 0) {
                continue;
            }

            int t = tonewheel(k, d);
            invR[t] += drawbar_volume(drawbars[d]) / resistance(k, d);
        }
    }

    float invmax = 1.0 / manual_max_volume();
    for (int t = 1; t < 92; t++) {
        float current = voltages[t] * invR[t];
        ret[t] = (uint16_t)(invmax * current * (float)(1 << 14) + 0.5);
    }
}

#if defined(__cplusplus)
}
#endif

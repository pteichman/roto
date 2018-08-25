/* Copyright (c) 2018 Peter Teichman */

#ifdef ROTO_TEST

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "greatest.h"

#include "manual.h"

TEST test_manual_foldback() {
    // Lower foldback: wrap tonewheels < 13 back into the 13..92 range.
    for (int i = 1; i < 13; i++) {
        ASSERT_EQ_FMT(i + 12, foldback(i), "%d");
    }

    // No foldback applied up through tonewheel 92.
    for (int i = 13; i < 92; i++) {
        ASSERT_EQ_FMT(i, foldback(i), "%d");
    }

    // Upper foldback: the first octave above 91 is mapped down one octave.
    for (int i = 92; i < 104; i++) {
        ASSERT_EQ_FMT(i - 12, foldback(i), "%d");
    }

    // Upper foldback: the next octave is mapped down two octaves.
    for (int i = 104; i < 116; i++) {
        ASSERT_EQ_FMT(i - 24, foldback(i), "%d");
    }

    PASS();
}

// test_manual_tonewheel ensures each key+drawbar combination is
// connected to the correct tonewheel.
TEST test_manual_tonewheel() {
    // key 1 drawbar 1 -> tonewheel 13 (foldback)
    ASSERT_EQ_FMT(13, tonewheel(1, 1), "%d");
    ASSERT_EQ_FMT(20, tonewheel(1, 2), "%d");
    ASSERT_EQ_FMT(13, tonewheel(1, 3), "%d");
    ASSERT_EQ_FMT(25, tonewheel(1, 4), "%d");
    ASSERT_EQ_FMT(32, tonewheel(1, 5), "%d");
    ASSERT_EQ_FMT(37, tonewheel(1, 6), "%d");
    ASSERT_EQ_FMT(41, tonewheel(1, 7), "%d");
    ASSERT_EQ_FMT(44, tonewheel(1, 8), "%d");
    ASSERT_EQ_FMT(49, tonewheel(1, 9), "%d");

    PASS();
}

// The tonewheel oscillator block expects these volumes to be
// Q19. This test ensures we never overflow that range.
TEST test_manual_volume_overflow() {
    uint8_t keys[62] = {0};
    uint8_t drawbars[10] = {0};
    uint16_t ret[92] = {0};

    // Hold down all the keys on this manual.
    for (int i = 0; i < 62; i++) {
        keys[i] = 1;
    }

    // Pull out all the stops.
    for (int i = 0; i < 10; i++) {
        drawbars[i] = 8;
    }

    uint32_t total = manual_fill_volumes(keys, drawbars, ret);

    // The total volumes here cannot overflow the expected Q19 range.
    uint32_t sum = 0;
    for (int i = 0; i < 92; i++) {
        sum += ret[i];
    }
    ASSERT_EQ_FMT(total, sum, "%d");

    if (sum > (1 << 19)) {
        char *msg = calloc(128, 1);
        snprintf(msg, 128, "overflow volume=%d (limit %d)", sum, (1 << 19));
        FAILm(msg);
        free(msg);
    }

    PASS();
}

// Our core sine oscillator has Q12 bits of precision. To avoid
// truncating its precision, volume output must be at least 1<<10.
// This ensures we only scale those sines up rather than down.
TEST test_manual_volume_underflow() {
    uint8_t keys[62] = {0};
    uint8_t drawbars[10] = {0};
    uint16_t ret[92] = {0};

    // Track the key and drawbar where the minimum volume is found.
    int mink = -1;
    int mind = -1;
    int mint = -1;
    int minv = SHRT_MAX;

    for (int k = 13; k < 62; k++) {
        // Hold down one key.
        memset(keys, 0, 62);
        keys[k] = 1;

        for (int d = 1; d < 10; d++) {
            // Pull one drawbar out one stop.
            memset(drawbars, 0, 10);
            drawbars[d] = 1;

            manual_fill_volumes(keys, drawbars, ret);
            for (int t = 1; t < 92; t++) {
                if (ret[t] == 0 || ret[t] >= minv) {
                    continue;
                }

                mink = k;
                mind = d;
                mint = t;
                minv = ret[t];
            }
        }
    }

    // Enforce volumes of at least 128 so we don't truncate the
    // precision of our sine waves too much.
    if (minv < (1 << 7)) {
        char *msg = calloc(128, 1);
        snprintf(msg, 128, "underflow k=%d d=%d t=%d -> volume=%d", mink, mind, mint, minv);
        FAILm(msg);
        free(msg);
    }

    PASS();
}

GREATEST_SUITE(manual_suite) {
    RUN_TEST(test_manual_foldback);
    RUN_TEST(test_manual_tonewheel);
    RUN_TEST(test_manual_volume_overflow);
    RUN_TEST(test_manual_volume_underflow);
}

#endif

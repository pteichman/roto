/* Copyright (c) 2018 Peter Teichman */

#ifdef ROTO_TEST

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "greatest.h"

#include "amfm.h"

TEST test_fill_sinemod() {
    int16_t vals[256] = {0};
    fill_sinemod(vals, -127, 127, 0);

    // Check our two zero crossings.
    ASSERT_EQ(0, vals[0]);
    ASSERT_EQ(0, vals[128]);

    // And our peaks.
    ASSERT_EQ(127, vals[64]);
    ASSERT_EQ(-127, vals[192]);

    for (int i = 0; i < 256; i++) {
        ASSERTm("underflow", vals[i] >= -127);
        ASSERTm("overflow", vals[i] <= 127);
    }

    PASS();
}

TEST test_fill_sinemod_zeros() {
    int16_t vals[256] = {0};
    fill_sinemod(vals, 0, 0, 0);

    for (int i = 0; i < 256; i++) {
        ASSERT_EQ(0, vals[i]);
    }

    PASS();
}

TEST test_fill_sinemod_phase() {
    int16_t vals[256] = {0};

    // Shift the sine by a quarter turn: now it's a cosine.
    fill_sinemod(vals, -127, 127, (1 << 15) / 4);

    // Check our two zero crossings.
    ASSERT_EQ(0, vals[64]);
    ASSERT_EQ(0, vals[192]);

    // And our peaks.
    ASSERT_EQ(127, vals[0]);
    ASSERT_EQ(-127, vals[128]);

    for (int i = 0; i < 256; i++) {
        ASSERTm("underflow", vals[i] >= -127);
        ASSERTm("overflow", vals[i] <= 127);
    }

    PASS();
}

TEST test_fill_sinemod_amplitude() {
    int16_t vals[256] = {0};
    fill_sinemod(vals, 0, 127, 0);

    for (int i = 0; i < 256; i++) {
        ASSERTm("underflow", vals[i] >= 0);
        ASSERTm("overflow", vals[i] <= 127);
    }

    PASS();
}

GREATEST_SUITE(amfm_suite) {
    RUN_TEST(test_fill_sinemod);
    RUN_TEST(test_fill_sinemod_zeros);
    RUN_TEST(test_fill_sinemod_phase);
    RUN_TEST(test_fill_sinemod_amplitude);
}

#endif

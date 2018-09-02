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

TEST test_amfm_update() {
    // Make sure a constant input results in a constant output.
    int srcdst_len = 8;
    int16_t src[8] = {1024, 1014, 1024, 1024, 1024, 1024, 1024, 1024};
    int16_t dst[8] = {0};

    int16_t ringbuf[512] = {0};
    int ringbuf_len = 512;

    int16_t volume[257] = {0};
    for (int i = 0; i < 257; i++) {
        volume[i] = (1 << 15) - 1;
    }

    int16_t offset[257] = {0};
    fill_sinemod(offset, 0, (int16_t)(44.1 * 256), 0); // 0ms -> ~1ms delay (44100 / 1000)
    offset[256] = offset[0];

    int wp = 0;
    uint32_t phase = 0;

    amfm_update(dst, src, srcdst_len, ringbuf, ringbuf_len, &wp, volume, offset, 1 << 24, &phase);

    PASS();
}

GREATEST_SUITE(amfm_suite) {
    RUN_TEST(test_fill_sinemod);
    RUN_TEST(test_fill_sinemod_zeros);
    RUN_TEST(test_fill_sinemod_phase);
    RUN_TEST(test_fill_sinemod_amplitude);
    RUN_TEST(test_amfm_update);
}

#endif

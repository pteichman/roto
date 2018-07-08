/* Copyright (c) 2018 Peter Teichman */

#ifdef ROTO_TEST

#include "greatest.h"

#include "tonewheel_osc.h"

TEST test_tonewheel_osc_new() {
    tonewheel_osc *osc = tonewheel_osc_new();

    ASSERT_EQ_FMT(0, osc->phase_incrs[0], "%d");
    for (int i = 0; i < 92; i++) {
        ASSERT_EQ_FMT(0, osc->phases[i], "%d");
        ASSERT_EQ_FMT(0, osc->volumes[i], "%d");
    }
    free(osc);
    PASS();
}

TEST test_tonewheel_osc_fill1() {
    tonewheel_osc *osc = tonewheel_osc_new();
    tonewheel_osc_set_volume(osc, 46, 100);

    int16_t block[10];
    tonewheel_osc_fill(osc, &block[0], 10);

    int16_t want[10];
    want[0] = 0x0019;
    want[1] = 0x0032;
    want[2] = 0x004b;
    want[3] = 0x0063;
    want[4] = 0x007c;
    want[5] = 0x0093;
    want[6] = 0x00aa;
    want[7] = 0x00c1;
    want[8] = 0x00d7;
    want[9] = 0x00eb;

    ASSERT_MEM_EQ(want, block, 10 * sizeof(int16_t));

    free(osc);
    PASS();
}

GREATEST_SUITE(tonewheel_osc_suite) {
    RUN_TEST(test_tonewheel_osc_new);
    RUN_TEST(test_tonewheel_osc_fill1);
}

#endif

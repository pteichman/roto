#ifdef ROTO_TEST

/* Copyright (c) 2018 Peter Teichman */

#include "greatest.h"

#include "tonewheel_osc.h"

TEST test_tonewheel_osc_new() {
    tonewheel_osc *osc = tonewheel_osc_new();

    ASSERT_EQ_FMT(0, osc->phase_incrs[0], "%d");
    for (int i=0; i<92; i++) {
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
    want[0] = 0x0101;
    want[1] = 0x0202;
    want[2] = 0x0302;
    want[3] = 0x03fe;
    want[4] = 0x04f6;
    want[5] = 0x05e9;
    want[6] = 0x06d6;
    want[7] = 0x07bc;
    want[8] = 0x089a;
    want[9] = 0x096f;

    ASSERT_MEM_EQ(want, block, 10*sizeof(int16_t));

    free(osc);
    PASS();
}

GREATEST_SUITE(tonewheel_osc_suite) {
    RUN_TEST(test_tonewheel_osc_new);
    RUN_TEST(test_tonewheel_osc_fill1);
}

#endif
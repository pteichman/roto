/* Copyright (c) 2018 Peter Teichman */

#ifdef ROTO_TEST

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

GREATEST_SUITE(manual_suite) {
    RUN_TEST(test_manual_foldback);
    RUN_TEST(test_manual_tonewheel);
}

#endif

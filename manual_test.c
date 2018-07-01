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

GREATEST_SUITE(manual_suite) {
    RUN_TEST(test_manual_foldback);
}

#endif

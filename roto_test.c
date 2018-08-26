/* Copyright (c) 2018 Peter Teichman */

#ifdef ROTO_TEST

#include "greatest.h"

extern SUITE(amfm_suite);
extern SUITE(manual_suite);
extern SUITE(tonewheel_osc_suite);

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();

    RUN_SUITE(amfm_suite);
    RUN_SUITE(manual_suite);
    RUN_SUITE(tonewheel_osc_suite);

    GREATEST_MAIN_END();
}

#endif

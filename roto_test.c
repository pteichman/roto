#ifdef ROTO_TEST

#include "greatest.h"

extern SUITE(tonewheel_osc_suite);

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();

    RUN_SUITE(tonewheel_osc_suite);

    GREATEST_MAIN_END();
}

#endif

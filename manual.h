/* Copyright (c) 2018 Peter Teichman */

#ifndef MANUAL_H
#define MANUAL_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

// manual simulates an organ keyboard and its drawbars.
typedef struct _manual {
    uint8_t drawbars[9];
    uint8_t keys[61];

    uint16_t output[92];
} manual;

int foldback(uint8_t tonewheel);

#if defined(__cplusplus)
}
#endif

#endif

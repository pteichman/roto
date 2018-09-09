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

uint32_t manual_fill_volumes(uint8_t keys[62], uint8_t drawbars[10], uint16_t ret[92]);
uint8_t manual_quantize_drawbar(uint8_t val);

int foldback(uint8_t tonewheel);
int tonewheel(int key, int drawbar);

#if defined(__cplusplus)
}
#endif

#endif

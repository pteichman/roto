/* Copyright (c) 2011 Peter Teichman */

#ifndef _TONEWHEELS_H
#define _TONEWHEELS_H

#include <Arduino.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void tonewheels_init();
void tonewheels_sample_v(int16_t *samples, uint8_t count);

void tonewheels_key_down(uint8_t key);
void tonewheels_key_up(uint8_t key);

void tonewheels_set_drawbar(uint8_t drawbar, uint8_t value);

#ifdef __cplusplus
}
#endif

#endif /* _TONEWHEELS_H */

/* Copyright (c) 2011 Peter Teichman */

#ifndef _TONEWHEELS_H
#define _TONEWHEELS_H

void tonewheels_init();
void tonewheels_sample_v(uint16_t *samples, uint8_t count);

void tonewheels_key_down(uint8_t key);
void tonewheels_key_up(uint8_t key);

void tonewheels_set_drawbar(uint8_t drawbar, uint8_t value);

#endif /* _TONEWHEELS_H */

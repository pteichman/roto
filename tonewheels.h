/* Copyright (c) 2011 Peter Teichman */

#ifndef _TONEWHEELS_H
#define _TONEWHEELS_H

uint16_t organ_sample();

extern const uint8_t sineTable[];
extern uint8_t tonewheel_volumes[];
extern const uint16_t tonewheel_rates[];
extern uint16_t tonewheel_positions[];

void tonewheels_init();
uint16_t tonewheels_sample();

#endif /* _TONEWHEELS_H */

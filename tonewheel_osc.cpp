/* Copyright (c) 2018 Peter Teichman */

extern "C" {

#include <stdlib.h>
#include <string.h>

#include "tonewheel_osc.h"

uint32_t freq_incr32(float freq);
int32_t isin_S3(int32_t x);
int32_t isin_S4(int32_t x);

float freqs[92] = {
    // Add an empty first element so freqs can be 1-indexed, like
    // Hammond docs online.
    0,
    32.6923,
    34.6341,
    36.7123,
    38.8889,
    41.2000,
    43.6364,
    46.2500,
    49.0000,
    51.8919,
    55.0000,
    58.2609,
    61.7143,
    65.3846,
    69.2683,
    73.4247,
    77.7778,
    82.4000,
    87.2727,
    92.5000,
    98.0000,
    103.7838,
    110.0000,
    116.5217,
    123.4286,
    130.7692,
    138.5366,
    146.8493,
    155.5556,
    164.8000,
    174.5455,
    185.0000,
    196.0000,
    207.5676,
    220.0000,
    233.0435,
    246.8571,
    261.5385,
    277.0732,
    293.6986,
    311.1111,
    329.6000,
    349.0909,
    370.0000,
    392.0000,
    415.1351,
    440.0000,
    466.0870,
    493.7143,
    523.0769,
    554.1463,
    587.3973,
    622.2222,
    659.2000,
    698.1818,
    740.0000,
    784.0000,
    830.2703,
    880.0000,
    932.1739,
    987.4286,
    1046.1538,
    1108.2927,
    1174.7945,
    1244.4444,
    1318.4000,
    1396.3636,
    1480.0000,
    1568.0000,
    1660.5405,
    1760.0000,
    1864.3478,
    1974.8571,
    2092.3077,
    2216.5854,
    2349.5890,
    2488.8889,
    2636.8000,
    2792.7273,
    2960.0000,
    3136.0000,
    3321.0811,
    3520.0000,
    3728.6957,
    3949.7143,
    4189.0909,
    4440.0000,
    4704.0000,
    4981.6216,
    5280.0000,
    5593.0435,
    5924.5714,
};

tonewheel_osc *tonewheel_osc_new() {
    tonewheel_osc *ret = (tonewheel_osc *)calloc(1, sizeof(tonewheel_osc));

    for (int i=0; i<92; i++) {
	ret->phase_incrs[i] = freq_incr32(freqs[i]);
    }

    return ret;
};

// freq_incr32 returns a 32-bit phase increment for freq at a sample
// rate of 44100kHz.
uint32_t freq_incr32(float freq) {
    // 1<<32 / 44100 = 97391.5
    return (uint32_t)(freq * 97391.5 + 0.5);
}

uint32_t freq_incr15(float freq) {
    // 1<<15 / 44100 = 0.74304
    return (uint32_t)(freq * 0.74304 + 0.5);
}

void tonewheel_osc_set_volume(tonewheel_osc *osc, uint8_t tonewheel, uint8_t volume) {
    if (tonewheel > 0 && tonewheel < 92) {
	osc->volumes[tonewheel] = volume;
    }
}

int8_t lookup(int8_t *table, uint32_t phase) {
    // Turn the phase into an 8-bit index.
    int index = phase >> 24;
    return table[index];
}

void tonewheel_osc_fill(tonewheel_osc *osc, int16_t *block, size_t block_len) {
    memset(block, 0, sizeof(int16_t)*block_len);

    uint32_t phase;
    uint32_t phase_incr;
    uint8_t volume;

    for (int i=0; i<92; i++) {
	phase = osc->phases[i];
	phase_incr = freq_incr15(freqs[i]);
	volume = osc->volumes[i];

	if (volume == 0) {
	    continue;
	}

	for (int j=0; j<block_len; j++) {
	    phase += phase_incr;
	    block[j] += (int16_t)isin_S4(phase);
	}
	osc->phases[i] = phase;
    }
}

/// A sine approximation via a third-order approx.
/// @param x    Angle (with 2^15 units/circle)
/// @return     Sine value (Q12)
int32_t isin_S3(int32_t x)
{
    // S(x) = x * ( (3<<p) - (x*x>>r) ) >> s
    // n : Q-pos for quarter circle             13
    // A : Q-pos for output                     12
    // p : Q-pos for parentheses intermediate   15
    // r = 2n-p                                 11
    // s = A-1-p-n                              17

    static const int qN = 13, qA= 12, qP= 15, qR= 2*qN-qP, qS= qN+qP+1-qA;

    x= x<<(30-qN);          // shift to full s32 range (Q13->Q30)

    if( (x^(x<<1)) < 0)     // test for quadrant 1 or 2
        x= (1<<31) - x;

    x= x>>(30-qN);

    return x * ( (3<<qP) - (x*x>>qR) ) >> qS;
}

/// A sine approximation via a fourth-order cosine approx.
/// @param x   angle (with 2^15 units/circle)
/// @return     Sine value (Q12)
int32_t isin_S4(int32_t x)
{
    int c, x2, y;
    static const int qN= 13, qA= 12, B=19900, C=3516;

    c= x<<(30-qN);              // Semi-circle info into carry.
    x -= 1<<qN;                 // sine -> cosine calc

    x= x<<(31-qN);              // Mask with PI
    x= x>>(31-qN);              // Note: SIGNED shift! (to qN)
    x= x*x>>(2*qN-14);          // x=x^2 To Q14

    y= B - (x*C>>14);           // B - x^2*C
    y= (1<<qA)-(x*y>>16);       // A - x^2*(B-x^2*C)

    return c>=0 ? y : -y;
}

} // end extern "C"

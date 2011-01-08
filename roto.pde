/* Copyright (c) 2011 Peter Teichman */

#include "Midi.h"

#include "tonewheels.h"

#define BUFFER_LEN 150

uint16_t buffer1[BUFFER_LEN];
uint16_t buffer2[BUFFER_LEN];
uint16_t *cur = 0;
uint16_t cur_sample;

volatile boolean gen_buffer1;
volatile boolean gen_buffer2;

static const uint8_t harmonics[] = { 0, 0, 19, 12, 24, 31, 36, 40, 43, 48 };

class RotoMidi : public Midi {
 public:
 RotoMidi(HardwareSerial &s) : Midi(s) { }

    uint8_t getDrawbarTonewheel(uint8_t key, uint8_t drawbar) {
        uint8_t tonewheel = key + harmonics[drawbar];

        if (tonewheel < 13) {
            tonewheel = tonewheel + 12;
        } else if (tonewheel > 103) {
            tonewheel = tonewheel - 24;
        } else if (tonewheel > 91) {
            tonewheel = tonewheel - 12;
        }

        return tonewheel;
    }

    void handleNoteOn(unsigned int channel, unsigned int note,
                      unsigned int velocity) {
        uint8_t midiNote = note - 12;

        bitWrite(PORTD, 7, 1);

        tonewheel_volumes[getDrawbarTonewheel(midiNote, 1)] += 16;
        tonewheel_volumes[getDrawbarTonewheel(midiNote, 2)] += 16;
        tonewheel_volumes[getDrawbarTonewheel(midiNote, 4)] += 8;
    }

    void subtractVolume(uint8_t tonewheel, uint8_t volume) {
        tonewheel_volumes[tonewheel] -= volume;

        if (tonewheel_volumes[tonewheel] == 0) {
            tonewheel_positions[tonewheel] = -tonewheel_rates[tonewheel];
        }
    }

    void handleNoteOff(unsigned int channel, unsigned int note,
                      unsigned int velocity) {
        uint8_t midiNote = note - 12;
        bitWrite(PORTD, 7, 0);

        subtractVolume(getDrawbarTonewheel(midiNote, 1), 16);
        subtractVolume(getDrawbarTonewheel(midiNote, 2), 16);
        subtractVolume(getDrawbarTonewheel(midiNote, 4), 8);
    }
};

RotoMidi midi(Serial);

void setup() {
    tonewheels_init();
    midi.begin(0);

    pinMode(7, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(4, OUTPUT);

    /* Enable interrupt on timer2 == 127, with clk/8 prescaler. At 16MHz,
       this gives a timer interrupt at 15625Hz. */
    TIMSK2 = (1 << OCIE2A);
    OCR2A = 127;

    /* clear/reset timer on match */
    TCCR2A = 1<<WGM21 | 0<<WGM20; /* CTC mode, reset on match */
    TCCR2B = 0<<CS22 | 1<<CS21 | 0<<CS20; /* clk, no prescaler */

    SPCR = 0x50;
    SPSR = 0x01;
    DDRB |= 0x2E;
    PORTB |= (1<<1);

    tonewheels_sample_v(&buffer1[0], BUFFER_LEN);
    tonewheels_sample_v(&buffer2[0], BUFFER_LEN);
    cur = &buffer1[0];

    sei();
}

ISR(TIMER2_COMPA_vect) {
    /* OCR2A has been cleared, per TCCR2A above */
    OCR2A = 127;

    cur_sample = (*cur++) >> 4;

    /* buffered, 1x gain, active mode */
    PORTB &= ~(1<<1);

    SPDR = highByte(cur_sample) | 0x70;
    while (!(SPSR & (1<<SPIF)));

    SPDR = lowByte(cur_sample);
    while (!(SPSR & (1<<SPIF)));

    PORTB |= (1<<1);

    if (cur == &buffer1[BUFFER_LEN]) {
        /* regenerate the first buffer once we're through with it */
        gen_buffer1 = true;
        cur = &buffer2[0];
        bitWrite(PORTD, 4, 0);
    } else if (cur == &buffer2[BUFFER_LEN]) {
        /* regenerate the second buffer */
        gen_buffer2 = true;
        cur = &buffer1[0];
        bitWrite(PORTD, 4, 1);
    }
}

void loop() {
    bitWrite(PORTD, 6, 0);
    bitWrite(PORTD, 6, 1);

    if (gen_buffer1) {
        tonewheels_sample_v(&buffer1[0], BUFFER_LEN);
        gen_buffer1 = false;
    } else if (gen_buffer2) {
        tonewheels_sample_v(&buffer2[0], BUFFER_LEN);
        gen_buffer2 = false;
    }

    midi.poll();

    bitWrite(PORTD, 6, 0);
}

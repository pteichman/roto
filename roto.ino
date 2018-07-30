/* Copyright (c) 2018 Peter Teichman */

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include "manual.h"
#include "preamp_audio.h"
#include "tonewheel_osc_audio.h"
#include "vibrato_audio.h"

// Hammond B-3.
AudioMixer4 organOut;
TonewheelOsc tonewheels;
Vibrato vibrato;
AudioConnection patchCord0(tonewheels, 0, vibrato, 0);
AudioConnection patchCord1(vibrato, 0, organOut, 0);

AudioSynthWaveformSine percussion;
AudioEffectEnvelope percussionEnv;
AudioConnection patchCord2(percussion, 0, percussionEnv, 0);
AudioConnection patchCord3(percussionEnv, 0, organOut, 1);

// Leslie 122.
Preamp preamp;
AudioConnection patchCord4(organOut, 0, preamp, 0);

// Teensy output.
AudioFilterBiquad antialias;
AudioConnection patchCord5(organOut, 0, antialias, 0);

AudioOutputI2S i2s1;
AudioControlSGTL5000 audioShield;
AudioConnection patchCord6(antialias, 0, i2s1, 0);
AudioConnection patchCord7(antialias, 0, i2s1, 1);

uint8_t keys[62] = {0};
uint8_t drawbars[10] = {0};
uint16_t volumes[92] = {0};

uint8_t numKeysDown = 0;
void handleNoteOn(byte chan, byte note, byte vel);
void handleNoteOff(byte chan, byte note, byte vel);

void setup() {
    Serial.begin(115200);

    AudioMemory(10);

    tonewheels.init();
    vibrato.init();
    preamp.init();

    percussion.amplitude(1.0);
    percussion.frequency(440);
    percussion.phase(0);

    percussionEnv.delay(0.0);
    percussionEnv.attack(0.0);
    percussionEnv.decay(300.0);
    percussionEnv.sustain(0.0);
    percussionEnv.release(0.0);

    organOut.gain(0, 0.95); // tonewheels + vibrato
    organOut.gain(1, 0.05); // percussionEnv
    organOut.gain(2, 0);
    organOut.gain(3, 0);

    drawbars[1] = 8;
    drawbars[2] = 8;
    drawbars[3] = 8;
    drawbars[4] = 8;

    vibrato.setMode(C3);
    antialias.setLowpass(0, 6000, 0.707);

    audioShield.enable();
    audioShield.volume(0.8);

    usbMIDI.begin();
    usbMIDI.setHandleControlChange(handleControlChange);
    usbMIDI.setHandleNoteOn(handleNoteOn);
    usbMIDI.setHandleNoteOff(handleNoteOff);
}

int count = 0;
void loop() {
    usbMIDI.read();
    if ((count++ % 500000) == 0) {
        usage();
    }
}

int note2key(byte note) {
    return (int)note - 35;
}

void fullPolyphony() {
    for (int n = 0; n < 128; n++) {
        handleNoteOn(1, n, 127);
    }
}

void randomDrawbars() {
    for (int d = 1; d < 10; d++) {
        drawbars[d] = random(0, 9);
        manual_fill_volumes(keys, drawbars, volumes);
        tonewheels.setVolumes(volumes);
    }
}

void handleNoteOn(byte chan, byte note, byte vel) {
    Serial.print("Note on: ");
    Serial.print(note);
    Serial.print("\n");

    int key = note2key(note);
    if (key < 1 || key > 61) {
        return;
    }

    if (++numKeysDown == 1) {
        float freq = pow(2.0, (note - 69) / 12.0) * 440;
        percussion.frequency(freq * 2);
        percussionEnv.noteOn();
    }
    keys[key] = 1;
    manual_fill_volumes(keys, drawbars, volumes);
    tonewheels.setVolumes(volumes);
}

void handleNoteOff(byte chan, byte note, byte vel) {
    Serial.print("Note off: ");
    Serial.print(note);
    Serial.print("\n");

    int key = note2key(note);
    if (key < 1 || key > 61) {
        return;
    }

    if (--numKeysDown == 0) {
        percussionEnv.noteOff();
    }
    keys[key] = 0;
    manual_fill_volumes(keys, drawbars, volumes);
    tonewheels.setVolumes(volumes);
}

void handleControlChange(byte chan, byte ctrl, byte val) {
    Serial.print("Control Change, ch=");
    Serial.print(chan, DEC);
    Serial.print(", control=");
    Serial.print(ctrl, DEC);
    Serial.print(", value=");
    Serial.print(val, DEC);
    Serial.println();

    if (ctrl >= 70 && ctrl < 79) {
        int drawbar = ctrl - 69;
        int pos = 0;
        if (val < 16) {
            pos = 0;
        } else if (val < 32) {
            pos = 1;
        } else if (val < 48) {
            pos = 2;
        } else if (val < 64) {
            pos = 3;
        } else if (val < 80) {
            pos = 4;
        } else if (val < 96) {
            pos = 5;
        } else if (val < 112) {
            pos = 6;
        } else if (val < 127) {
            pos = 7;
        } else {
            pos = 8;
        }

        if (drawbar == 4 /* && percussion is on */) {
            pos = 0;
        }

        drawbars[drawbar] = pos;
        manual_fill_volumes(keys, drawbars, volumes);
        tonewheels.setVolumes(volumes);
    }
}

void showKeys() {
    for (int i = 0; i < 62; i++) {
        Serial.print("keys[");
        Serial.print(i);
        Serial.print("] = ");
        Serial.print(keys[i]);
        Serial.print("\n");
    }
}

void showVolumes() {
    for (int i = 0; i < 92; i++) {
        Serial.print("volumes[");
        Serial.print(i);
        Serial.print("] = ");
        Serial.print(volumes[i]);
        Serial.print("\n");
    }
}

void usage() {
    Serial.print("CPU: ");
    Serial.print("tonewheels=");
    Serial.print(tonewheels.processorUsage());
    Serial.print(",");
    Serial.print(tonewheels.processorUsageMax());
    Serial.print("  ");

    Serial.print("vibrato=");
    Serial.print(vibrato.processorUsage());
    Serial.print(",");
    Serial.print(vibrato.processorUsageMax());
    Serial.print("  ");

    Serial.print("preamp=");
    Serial.print(preamp.processorUsage());
    Serial.print(",");
    Serial.print(preamp.processorUsageMax());
    Serial.print("  ");

    Serial.print("antialias=");
    Serial.print(antialias.processorUsage());
    Serial.print(",");
    Serial.print(antialias.processorUsageMax());
    Serial.print("  ");

    Serial.print("all=");
    Serial.print(AudioProcessorUsage());
    Serial.print(",");
    Serial.print(AudioProcessorUsageMax());
    Serial.print("    ");

    Serial.print("Memory: ");
    Serial.print(AudioMemoryUsage());
    Serial.print(",");
    Serial.print(AudioMemoryUsageMax());
    Serial.print("    ");
    Serial.println();
}

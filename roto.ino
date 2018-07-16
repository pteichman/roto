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

TonewheelOsc tonewheels;
Vibrato vibrato;
Preamp preamp;
AudioFilterBiquad antialias;
AudioOutputI2S i2s1;

AudioConnection patchCord0(tonewheels, 0, vibrato, 0);
AudioConnection patchCord1(vibrato, 0, preamp, 0);
AudioConnection patchCord2(preamp, 0, antialias, 0);
AudioConnection patchCord3(antialias, 0, i2s1, 0);
AudioConnection patchCord4(antialias, 0, i2s1, 1);
AudioControlSGTL5000 audioShield;

uint8_t keys[62] = {0};
uint8_t drawbars[10] = {0};
uint16_t volumes[92] = {0};

void handleNoteOn(byte chan, byte note, byte vel);
void handleNoteOff(byte chan, byte note, byte vel);

void setup() {
    Serial.begin(115200);

    AudioMemory(10);

    tonewheels.init();
    vibrato.init();
    preamp.init();

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

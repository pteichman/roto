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

TonewheelOsc percussion;
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
uint16_t percVolumes[92] = {0};

// Current state of the percussion settings.
uint8_t percOn = 1;
uint8_t percThird = 0;
uint8_t percFast = 0;
uint8_t percSoft = 0;

// numKeysDown is used to keep the percussion effect single triggered:
// only the first key down affects the percussion setting.
uint8_t numKeysDown = 0;

void handleNoteOn(byte chan, byte note, byte vel);
void handleNoteOff(byte chan, byte note, byte vel);
void handleControlChange(byte chan, byte ctrl, byte val);

void setup() {
    Serial.begin(115200);

    AudioMemory(10);

    tonewheels.init();
    percussion.init();
    vibrato.init();
    preamp.init();

    drawbars[1] = 8;
    drawbars[2] = 8;
    drawbars[3] = 8;
    drawbars[4] = 8;

    updatePercussion();
    updateTonewheels();

    organOut.gain(0, 0.50); // tonewheels + vibrato
    organOut.gain(1, 0.50); // percussionEnv
    organOut.gain(2, 0);
    organOut.gain(3, 0);

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
        status();
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
        updateTonewheels();
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
    updateTonewheels();

    if (++numKeysDown == 1 && percOn) {
        percussionEnv.noteOn();
    }
}

void handleNoteOff(byte chan, byte note, byte vel) {
    Serial.print("Note off: ");
    Serial.print(note);
    Serial.print("\n");

    int key = note2key(note);
    if (key < 1 || key > 61) {
        return;
    }

    if (--numKeysDown == 0 && percOn) {
        percussionEnv.noteOff();
    }

    keys[key] = 0;
    updateTonewheels();
}

void updatePercussion() {
    percussionEnv.delay(0.0);
    percussionEnv.attack(0.0);
    percussionEnv.sustain(0.0);
    percussionEnv.release(0.0);

    if (percFast) {
        percussionEnv.decay(300.0);
    } else {
        percussionEnv.decay(630.0);
    }

    if (percSoft) {
        organOut.gain(1, 0.25);
    } else {
        organOut.gain(1, 0.50);
    }
}

void updateTonewheels() {
    // Copy the drawbars so we can tweak them based on percussion
    // settings (enabling percussion steals a drawbar).
    uint8_t bars[10] = {0};
    memcpy(bars, drawbars, 10);
    uint8_t percBars[10] = {0};

    if (percOn) {
        bars[9] = 0;
        if (percThird) {
            percBars[5] = 32;
        } else {
            percBars[4] = 32;
        }
    }

    manual_fill_volumes(keys, percBars, percVolumes);
    percussion.setVolumes(percVolumes);

    manual_fill_volumes(keys, bars, volumes);
    tonewheels.setVolumes(volumes);
}

// handleControlChange is compatible (where possible) with the Nord
// Electro 3 MIDI implementation:
// http://www.nordkeyboards.com/sites/default/files/files/downloads/manuals/nord-electro-3/Nord%20Electro%203%20English%20User%20Manual%20v3.x%20Edition%203.1.pdf
void handleControlChange(byte chan, byte ctrl, byte val) {
    Serial.print("Control Change, ch=");
    Serial.print(chan, DEC);
    Serial.print(", control=");
    Serial.print(ctrl, DEC);
    Serial.print(", value=");
    Serial.print(val, DEC);
    Serial.println();

    if (ctrl == 87) {
        percOn = val;
        updatePercussion();
        updateTonewheels();
    }
    if (ctrl == 88) {
        percFast = val;
        updatePercussion();
    }
    if (ctrl == 89) {
        percSoft = val;
        updatePercussion();
    }

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
        updateTonewheels();
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

void status() {
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

void statusPerc() {
    Serial.print("percOn=");
    Serial.print(percOn);
    Serial.print("    ");
    Serial.print("percFast=");
    Serial.print(percFast);
    Serial.print("    ");
    Serial.print("percSoft=");
    Serial.print(percSoft);
    Serial.print("    ");
    Serial.print("percThird=");
    Serial.print(percThird);
    Serial.print("    ");
    Serial.println();
}

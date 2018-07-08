#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include "manual.h"
#include "tonewheel_osc_audio.h"

TonewheelOsc tonewheels;
AudioFilterBiquad antialias;
AudioOutputI2S i2s1;

AudioConnection patchCord0(tonewheels, 0, antialias, 0);
AudioConnection patchCord1(antialias, 0, i2s1, 0);
AudioConnection patchCord2(antialias, 0, i2s1, 1);
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

    drawbars[1] = 8;
    drawbars[2] = 8;
    drawbars[3] = 8;
    drawbars[4] = 8;

    antialias.setLowpass(0, 6000, 0.707);

    audioShield.enable();
    audioShield.volume(0.8);

    usbMIDI.begin();
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

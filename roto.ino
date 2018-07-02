#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include "manual.h"
#include "tonewheel_osc_audio.h"

// GUItool: begin automatically generated code
TonewheelOsc tonewheels; //xy=55,20
AudioOutputI2S i2s1;     //xy=222,22
AudioFilterBiquad antialias;

AudioConnection patchCord0(tonewheels, 0, antialias, 0);
AudioConnection patchCord1(antialias, 0, i2s1, 0);
AudioConnection patchCord2(antialias, 0, i2s1, 1);
AudioControlSGTL5000 audioShield;
// GUItool: end automatically generated code

uint8_t keys[62] = {0};
float drawbars[10] = {0};
uint16_t volumes[92] = {0};

void handleNoteOn(byte chan, byte note, byte vel);
void handleNoteOff(byte chan, byte note, byte vel);

void setup() {
    Serial.begin(115200);

    AudioMemory(10);

    tonewheels.init();

    for (int i = 0; i < 10; i++) {
        drawbars[i] = -1;
    }
    drawbars[3] = 0;
    drawbars[5] = 0;
    drawbars[8] = 0;

    antialias.setLowpass(0, 6000, 0.707);

    audioShield.enable();
    audioShield.volume(0.30);

    usbMIDI.begin();
    usbMIDI.setHandleNoteOn(handleNoteOn);
    usbMIDI.setHandleNoteOff(handleNoteOff);

    handleNoteOn(0, 25, 100);
}

void loop() {
    usbMIDI.read();
}

int note2key(byte note) {
    return (int)note - 35;
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

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

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

void setup() {
    Serial.begin(115200);

    AudioMemory(10);

    tonewheels.init();
    //
    //  for (int i=13; i<80; i++) {
    //    tonewheels.setVolume(i, 100);
    //  }

    //  tonewheels.setVolume(1, 200);
    tonewheels.setVolume(46 - 24, 200);
    tonewheels.setVolume(46 - 12, 200);
    tonewheels.setVolume(46, 100);
    tonewheels.setVolume(46 + 4, 100);
    tonewheels.setVolume(46 + 7, 100);
    tonewheels.setVolume(46 + 11, 100);
    //  tonewheels.setVolume(46+12, 100);
    //  tonewheels.setVolume(46+14, 100);

    antialias.setLowpass(0, 6000, 0.707);

    audioShield.enable();
    audioShield.volume(0.80);
}

void loop() {
    // put your main code here, to run repeatedly:
    // print a summary of the current & maximum usage
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

    delay(1000);
}

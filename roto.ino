/* Copyright (c) 2018 Peter Teichman */

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SerialFlash.h>

#include "amfm_audio.h"
#include "manual.h"
#include "monitor_audio.h"
#include "preamp_audio.h"
#include "tonewheel_osc_audio.h"
#include "vibrato_audio.h"

// Hammond B-3.
AudioMixer4 organOut;
TonewheelOsc tonewheels;
Monitor tonewheelsMonitor;
Vibrato vibrato;

AudioConnection patchCord0(tonewheels, 0, tonewheelsMonitor, 0);
AudioConnection patchCord1(tonewheelsMonitor, 0, vibrato, 0);
AudioConnection patchCord2(vibrato, 0, organOut, 0);

TonewheelOsc percussion;
AudioEffectEnvelope percussionEnv;

AudioConnection patchCord3(percussion, 0, percussionEnv, 0);
AudioConnection patchCord4(percussionEnv, 0, organOut, 1);

AudioAmplifier swell;
AudioConnection patchCord5(organOut, 0, swell, 0);

// This antialias filter is here to band limit the organ signal, in
// case key click transients are too high frequency, and also to give
// a slight reduction in key click.
AudioFilterBiquad antialias;
AudioConnection patchCord6(swell, 0, antialias, 0);

// Leslie 122
Preamp preamp;
AudioFilterStateVariable crossover;
AmFm leslieBassR;
AmFm leslieTrebleR;
AudioMixer4 leslieR;
AmFm leslieBassL;
AmFm leslieTrebleL;
AudioMixer4 leslieL;

AudioConnection patchCord7(antialias, 0, preamp, 0);
AudioConnection patchCord8(preamp, 0, crossover, 0);

AudioConnection patchCord9(crossover, 0, leslieBassR, 0);
AudioConnection patchCord10(crossover, 2, leslieTrebleR, 0);
AudioConnection patchCord11(leslieBassR, 0, leslieR, 0);
AudioConnection patchCord12(leslieTrebleR, 0, leslieR, 1);

AudioConnection patchCord13(crossover, 0, leslieBassL, 0);
AudioConnection patchCord14(crossover, 2, leslieTrebleL, 0);
AudioConnection patchCord15(leslieBassL, 0, leslieL, 0);
AudioConnection patchCord16(leslieTrebleL, 0, leslieL, 1);

// Teensy audio board output.
AudioOutputI2S i2s1;
AudioControlSGTL5000 audioShield;
AudioConnection patchCord17(leslieR, 0, i2s1, 0);
AudioConnection patchCord18(leslieL, 0, i2s1, 1);

#ifdef AUDIO_INTERFACE
// If the board is configured for USB audio, mirror the i2s output to USB.
AudioOutputUSB usbAudio;
AudioConnection patchCord19(leslieR, 0, usbAudio, 0);
AudioConnection patchCord20(leslieL, 0, usbAudio, 1);
#endif

// MIDI state. keys[n] will be nonzero if a key is down (value being
// the most recent velocity). control[n] is the most recent value of a
// control message.
//
// In both cases, the highest bit indicates whether the value is the
// result of a message (1 if yes, 0 if it hasn't changed since
// initialization.
uint8_t midiKeys[127] = {0};
uint8_t midiControl[127] = {0};

#define MANUAL_KEY_0 (35)
#define MANUAL_KEY_61 (MANUAL_KEY_0 + 61)

#define CC_SWELL (11)
#define CC_RESET (46)
#define CC_DRAWBAR_0 (69)
#define CC_DRAWBAR_9 (CC_DRAWBAR_0 + 9)
#define CC_ROTARY_SPEED (82)
#define CC_ROTARY_STOP (79)
#define CC_PERCUSSION (87)
#define CC_PERCUSSION_FAST (88)
#define CC_PERCUSSION_SOFT (89)
#define CC_PERCUSSION_THIRD (95) // is this correct?

// numKeysDown is used to keep the percussion effect single triggered:
// only the first key down affects the percussion setting.
uint8_t numKeysDown = 0;

void handleNoteOn(byte chan, byte note, byte vel);
void handleNoteOff(byte chan, byte note, byte vel);
void handleControlChange(byte chan, byte ctrl, byte val);

// reset restores everything to just-booted state:
// 1) it thinks all keys are up
// 2) drawbar registration is set to 888800000
// 3) percussion is off, vibrato is set to C1
// 4) the Leslie is set to slow
void reset() {
    // Release all keys and reset all control settings.
    memset(midiKeys, 0, 127);
    memset(midiControl, 0, 127);

    // Set drawbars to Green Onions.
    midiControl[CC_DRAWBAR_0 + 1] = 127;
    midiControl[CC_DRAWBAR_0 + 2] = 127;
    midiControl[CC_DRAWBAR_0 + 3] = 127;
    midiControl[CC_DRAWBAR_0 + 4] = 127;

    // Reset Leslie rotation position. Our R microphone leads the L by
    // 90 degrees.
    leslieBassL.setPhase(0);
    leslieTrebleL.setPhase(0);
    leslieBassR.setPhase(0.25);
    leslieTrebleR.setPhase(0.25);

    updateLeslieAmplifier();
    updateLeslieRotation();

    updatePercussionEnvelope();
    updateTonewheelVolumes();
}

void setup() {
    Serial.begin(115200);

    AudioMemory(10);

    leslieBassR.init();
    leslieTrebleR.init();
    leslieBassL.init();
    leslieTrebleL.init();

    tonewheels.init();
    percussion.init();
    vibrato.init();

    reset();

    swell.gain(1.0);

    organOut.gain(0, 0.50); // tonewheels + vibrato
    organOut.gain(1, 0.50); // percussionEnv
    organOut.gain(2, 0);
    organOut.gain(3, 0);

    leslieR.gain(0, 0.70); // bass
    leslieR.gain(1, 0.30); // treble
    leslieL.gain(0, 0.70); // bass
    leslieL.gain(1, 0.30); // treble

    vibrato.setMode(Off);
    antialias.setLowpass(0, 6000, 0.707);

    audioShield.enable();
    audioShield.volume(0.5);

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
        statusVolume();
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
    for (int i = 1; i <= 9; i++) {
        midiControl[CC_DRAWBAR_0 + i] = random(0, 127);
    }
    updateTonewheelVolumes();
}

void handleNoteOn(byte chan, byte note, byte velocity) {
    Serial.print("Note on: ");
    Serial.print(note);
    Serial.print("\n");

    // MIDI notes always have the high bit unset, but just in case.
    if (note & 0x80) {
        return;
    }

    midiKeys[note] = velocity;
    if (note <= MANUAL_KEY_0 || note > MANUAL_KEY_61) {
        return;
    }

    updateTonewheelVolumes();

    if (++numKeysDown == 1 && midiControl[CC_PERCUSSION]) {
        percussionEnv.noteOn();
    }
}

void handleNoteOff(byte chan, byte note, byte vel) {
    Serial.print("Note off: ");
    Serial.print(note);
    Serial.print("\n");

    if (note & 0x80) {
        return;
    }

    midiKeys[note] = 0;
    if (note <= MANUAL_KEY_0 || note > MANUAL_KEY_61) {
        return;
    }

    if (--numKeysDown == 0 && midiControl[CC_PERCUSSION]) {
        percussionEnv.noteOff();
    }

    updateTonewheelVolumes();
}

void updateReset() {
    if (midiControl[CC_RESET]) {
        midiControl[CC_RESET] = 0;
        reset();
    }
}

void updatePercussionEnvelope() {
    percussionEnv.delay(0.0);
    percussionEnv.attack(0.1);
    percussionEnv.sustain(0.0);
    percussionEnv.release(0.0);

    if (midiControl[CC_PERCUSSION_FAST]) {
        percussionEnv.decay(300.0);
    } else {
        percussionEnv.decay(630.0);
    }

    if (midiControl[CC_PERCUSSION_SOFT]) {
        organOut.gain(1, 0.25);
    } else {
        organOut.gain(1, 0.50);
    }
}

uint8_t bars[10] = {0};
uint16_t volumes[92] = {0};
uint8_t percBars[10] = {0};
uint16_t percVolumes[92] = {0};

void updateTonewheelVolumes() {
    for (int i = 1; i < 10; i++) {
        bars[i] = manual_quantize_drawbar(midiControl[CC_DRAWBAR_0 + i]);
    }

    if (midiControl[CC_PERCUSSION]) {
        bars[9] = 0;
        if (midiControl[CC_PERCUSSION_THIRD]) {
            percBars[5] = manual_quantize_drawbar(127);
        } else {
            percBars[4] = manual_quantize_drawbar(127);
        }
    }

    manual_fill_volumes(&midiKeys[MANUAL_KEY_0], percBars, percVolumes);
    percussion.setVolumes(percVolumes);

    manual_fill_volumes(&midiKeys[MANUAL_KEY_0], bars, volumes);
    tonewheels.setVolumes(volumes);
}

void updateLeslieAmplifier() {
    preamp.setK(50.0);
    crossover.frequency(800);
    crossover.resonance(0.707);
}

void updateLeslieRotation() {
    // Reset some things that should be constant.
    leslieBassR.setTremoloDepth(0.5);
    leslieTrebleR.setTremoloDepth(0.3);
    leslieBassL.setTremoloDepth(0.5);
    leslieTrebleL.setTremoloDepth(0.3);

    // These Leslie speeds are from
    // http://www.dairiki.org/HammondWiki/LeslieRotationSpeed

    if (midiControl[CC_ROTARY_STOP]) {
        // Stop
        leslieBassR.setRotationRate(0);
        leslieTrebleR.setRotationRate(0);
        leslieBassL.setRotationRate(0);
        leslieTrebleL.setRotationRate(0);
    } else if (midiControl[CC_ROTARY_SPEED]) {
        // Fast
        leslieBassR.setRotationRate(5.7);
        leslieTrebleR.setRotationRate(6.66);
        leslieBassL.setRotationRate(5.7);
        leslieTrebleL.setRotationRate(6.66);
    } else {
        // Slow
        leslieBassR.setRotationRate(0.666);
        leslieTrebleR.setRotationRate(0.8);
        leslieBassL.setRotationRate(0.666);
        leslieTrebleL.setRotationRate(0.8);
    }
}

float remap(float v, float oldmin, float oldmax, float newmin, float newmax) {
    return newmin + (v - oldmin) * (newmax - newmin) / (oldmax - oldmin);
}

// handleControlChange is compatible (where possible) with the Nord
// Electro 3 MIDI implementation:
// http://www.nordkeyboards.com/sites/default/files/files/downloads/manuals/nord-electro-3/Nord%20Electro%203%20English%20User%20Manual%20v3.x%20Edition%203.1.pdf
void handleControlChange(byte chan, byte ctrl, byte val) {
    if (ctrl == 1) {
        // Skip logging aftertouch messages, so the serial log isn't
        // spammed with them.
        return;
    }

    Serial.print("Control Change, ch=");
    Serial.print(chan, DEC);
    Serial.print(", control=");
    Serial.print(ctrl, DEC);
    Serial.print(", value=");
    Serial.print(val, DEC);
    Serial.println();

    if (ctrl & 0x80) {
        return;
    }

    midiControl[ctrl] = val;

    if (ctrl == CC_SWELL) {
        swell.gain(remap((float)val, 0, 127, 0, 2.5));
    } else if (ctrl == CC_RESET) {
        updateReset();
    } else if (ctrl == CC_PERCUSSION) {
        updatePercussionEnvelope();
        updateTonewheelVolumes();
    } else if (ctrl == CC_PERCUSSION_FAST) {
        updatePercussionEnvelope();
    } else if (ctrl == CC_PERCUSSION_SOFT) {
        updatePercussionEnvelope();
    } else if (ctrl > CC_DRAWBAR_0 && ctrl <= CC_DRAWBAR_9) {
        updateTonewheelVolumes();
    } else if (ctrl == CC_ROTARY_STOP || ctrl == CC_ROTARY_SPEED) {
	updateLeslieRotation();
    }
}

void showKeys() {
    for (int i = 0; i < 62; i++) {
        Serial.print("keys[");
        Serial.print(i);
        Serial.print("] = ");
        Serial.print(midiKeys[MANUAL_KEY_0 + i]);
        Serial.print("\n");
    }
}

void showVolumes(uint16_t volumes[92]) {
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

void statusVolume() {
    Serial.print("Volume: ");
    Serial.print("tonewheels=");
    Serial.print(tonewheelsMonitor.volumeUsageMin());
    Serial.print(",");
    Serial.print(tonewheelsMonitor.volumeUsageMax());
    Serial.print("    ");
    Serial.print(tonewheelsMonitor.volumeUsageMinEver());
    Serial.print(",");
    Serial.print(tonewheelsMonitor.volumeUsageMaxEver());
    Serial.println();

    tonewheelsMonitor.reset();
}

void statusPerc() {
    Serial.print("percOn=");
    Serial.print(midiControl[CC_PERCUSSION]);
    Serial.print("    ");
    Serial.print("percFast=");
    Serial.print(midiControl[CC_PERCUSSION_FAST]);
    Serial.print("    ");
    Serial.print("percSoft=");
    Serial.print(midiControl[CC_PERCUSSION_SOFT]);
    Serial.print("    ");
    Serial.print("percThird=");
    Serial.print(midiControl[CC_PERCUSSION_THIRD]);
    Serial.print("    ");
    Serial.println();
}

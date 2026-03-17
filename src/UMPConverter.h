#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

// MIDI 2.0 UMP Converter (32-bit per-note expression)
class UMPConverter
{
public:
    // Convert f0 + velocity to MIDI 2.0 UMP note packet
    // Returns raw 64-bit UMP word
    uint64_t noteOn(float f0Hz, float velocity, int channel);
    uint64_t noteOff(int note, int channel);
    uint64_t pitchBend32(float bendSemitones, int channel); // 32-bit vs MIDI1 14-bit

private:
    int freqToMidiNote(float hz);
    float midiNoteToFreq(int note);
};

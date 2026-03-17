#include "UMPConverter.h"
#include <cmath>

int UMPConverter::freqToMidiNote(float hz)
{
    return (int)std::round(69.0f + 12.0f * std::log2(hz / 440.0f));
}

float UMPConverter::midiNoteToFreq(int note)
{
    return 440.0f * std::pow(2.0f, (note - 69) / 12.0f);
}

uint64_t UMPConverter::noteOn(float f0Hz, float velocity, int channel)
{
    // MIDI 2.0 UMP Type 4 – MIDI 2.0 Channel Voice Message
    // Word 0: [MT=4][Group=0][Status=0x9][Channel][Note][Attribute]
    // Word 1: [Velocity 16-bit][AttributeData 16-bit]
    int note = freqToMidiNote(f0Hz);
    uint16_t vel16 = (uint16_t)(velocity * 65535.0f);

    uint32_t word0 = (0x40000000) | ((channel & 0xF) << 16) | ((note & 0x7F) << 8);
    uint32_t word1 = (vel16 << 16);

    return ((uint64_t)word0 << 32) | word1;
}

uint64_t UMPConverter::pitchBend32(float bendSemitones, int channel)
{
    // 32-bit pitchbend (vs MIDI1 14-bit) – true MIDI 2.0 UMP
    uint32_t bend32 = (uint32_t)((bendSemitones / 48.0f + 0.5f) * 4294967295.0f);
    uint32_t word0 = (0x40000000) | ((channel & 0xF) << 16) | (0x60 << 8);
    return ((uint64_t)word0 << 32) | bend32;
}

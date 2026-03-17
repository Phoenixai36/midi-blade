#pragma once
#include <juce_core/juce_core.h>

// MIDI 2.0 UMP Converter with full per-note expression + MIDI 1.0 fallback
class UMPConverter
{
public:
    UMPConverter();
    ~UMPConverter();
    
    // ==================== MIDI 2.0 Methods ====================
    
    // Note On with 16-bit velocity
    // Returns: Two 32-bit UMP words packed in uint64_t
    // Word 1: MT=2 | Group | Status=0x1 | Channel
    // Word 2: Note# | Velocity MSB | Velocity LSB
    uint64_t noteOn32(uint8_t note, uint16_t velocity16,
                       uint8_t group, uint8_t channel,
                       uint8_t attrType = 0, uint16_t attrData = 0);
    
    // Note Off with 16-bit velocity
    uint64_t noteOff32(uint8_t note, uint16_t velocity16,
                        uint8_t group, uint8_t channel);
    
    // Per-Note Pitch Bend (32-bit) - THE KEY FEATURE
    // Full 32-bit range: 0x00000000 to 0xFFFFFFFF
    // Center (no bend): 0x80000000
    uint64_t pitchBend32(uint8_t note, uint32_t bend32,
                          uint8_t group, uint8_t channel);
    
    // Per-Note Aftertouch (32-bit)
    uint64_t perNoteAT(uint8_t note, uint32_t pressure32,
                        uint8_t group, uint8_t channel);
    
    // Per-Note Management
    uint64_t perNoteManagement(uint8_t note, uint8_t flags,
                                uint8_t group, uint8_t channel);
    
    // ==================== MIDI 1.0 Fallback Methods ====================
    
    // Standard MIDI 1.0 Note On (combine velocity to 7-bit)
    uint32_t noteOn1(uint8_t note, uint8_t velocity7, uint8_t channel);
    
    // Standard MIDI 1.0 Note Off
    uint32_t noteOff1(uint8_t note, uint8_t velocity7, uint8_t channel);
    
    // MIDI 1.0 Pitch Bend (14-bit, compressed from 32-bit)
    uint32_t pitchBend1(uint8_t channel, uint16_t bend14);
    
    // MIDI 1.0 Channel Aftertouch (downsampled from 32-bit)
    uint32_t channelAT1(uint8_t channel, uint8_t pressure7);
    
    // ==================== Utility Methods ====================
    
    // Convert cents to 32-bit pitch bend value
    // Input: -4800 to +4800 cents (±48 semitones)
    // Output: 0x00000000 to 0xFFFFFFFF (center at 0x80000000)
    static uint32_t centsToPitchBend32(float cents);
    
    // Convert 32-bit pitch bend to 14-bit (for fallback)
    static uint16_t pitchBend32to14(uint32_t bend32);
    
    // Convert velocity16 to velocity7 (for fallback)
    static uint8_t velocity16to7(uint16_t vel16);
    
private:
    // UMP message type constants
    static constexpr uint8_t kMT_Midi1ChannelVoice = 0x2;
    static constexpr uint8_t kMT_Midi2ChannelVoice = 0x3;
    static constexpr uint8_t kStatus_NoteOn = 0x1;
    static constexpr uint8_t kStatus_NoteOff = 0x0;
    static constexpr uint8_t kStatus_PitchBend = 0x6;
    static constexpr uint8_t kStatus_PerNoteAT = 0xA;
    static constexpr uint8_t kStatus_PerNoteManagement = 0xE;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UMPConverter)
};

#include "UMPConverter.h"
#include <cmath>

UMPConverter::UMPConverter() = default;
UMPConverter::~UMPConverter() = default;

// ==================== MIDI 2.0 Implementation ====================

uint64_t UMPConverter::noteOn32(uint8_t note, uint16_t velocity16,
                                 uint8_t group, uint8_t channel,
                                 uint8_t attrType, uint16_t attrData)
{
    // Word 1: MT=2 | Group | Status=0x1 (NoteOn) | Channel
    uint8_t status = (kMT_Midi2ChannelVoice << 4) | (group & 0x0F);
    uint8_t word1 = (status << 4) | (kStatus_NoteOn << 0) | (channel & 0x0F);
    
    // Word 2: Note# | Velocity MSB | Velocity LSB
    uint8_t velMSB = static_cast<uint8_t>((velocity16 >> 8) & 0x7F);
    uint8_t velLSB = static_cast<uint8_t>(velocity16 & 0xFF);
    uint32_t word2 = (static_cast<uint32_t>(note) << 16) | 
                     (static_cast<uint32_t>(velMSB) << 8) | 
                     static_cast<uint32_t>(velLSB);
    
    // Word 3: Attr Type | Attr Data
    uint32_t word3 = (static_cast<uint32_t>(attrType) << 16) | 
                     static_cast<uint32_t>(attrData);
    
    // Pack into uint64_t (word1 in high 32 bits, word2 in low 32)
    // For 3-word messages, we need separate handling in consumer
    return (static_cast<uint64_t>(word1) << 32) | word2;
}

uint64_t UMPConverter::noteOff32(uint8_t note, uint16_t velocity16,
                                   uint8_t group, uint8_t channel)
{
    // Word 1: MT=2 | Group | Status=0x0 (NoteOff) | Channel
    uint8_t status = (kMT_Midi2ChannelVoice << 4) | (group & 0x0F);
    uint8_t word1 = (status << 4) | (kStatus_NoteOff << 0) | (channel & 0x0F);
    
    // Word 2: Note# | Velocity MSB | Velocity LSB
    uint8_t velMSB = static_cast<uint8_t>((velocity16 >> 8) & 0x7F);
    uint8_t velLSB = static_cast<uint8_t>(velocity16 & 0xFF);
    uint32_t word2 = (static_cast<uint32_t>(note) << 16) | 
                     (static_cast<uint32_t>(velMSB) << 8) | 
                     static_cast<uint32_t>(velLSB);
    
    return (static_cast<uint64_t>(word1) << 32) | word2;
}

uint64_t UMPConverter::pitchBend32(uint8_t note, uint32_t bend32,
                                    uint8_t group, uint8_t channel)
{
    // Word 1: MT=3 | Group | Status=0x6 (Per-Note Pitch Bend) | Channel
    uint8_t status = (kMT_Midi2ChannelVoice << 4) | (group & 0x0F);
    uint8_t word1 = (status << 4) | (kStatus_PitchBend << 0) | (channel & 0x0F);
    
    // Word 2: Note# | Reserved | Pitch Bend LSB | Pitch Bend MSB...
    // Full 32-bit bend in lower 4 bytes
    uint8_t noteB = note & 0x7F;
    uint32_t word2 = (static_cast<uint32_t>(noteB) << 24) | (bend32 & 0x00FFFFFF);
    
    return (static_cast<uint64_t>(word1) << 32) | word2;
}

uint64_t UMPConverter::perNoteAT(uint8_t note, uint32_t pressure32,
                                   uint8_t group, uint8_t channel)
{
    // Word 1: MT=3 | Group | Status=0xA (Per-Note AT) | Channel
    uint8_t status = (kMT_Midi2ChannelVoice << 4) | (group & 0x0F);
    uint8_t word1 = (status << 4) | (kStatus_PerNoteAT << 0) | (channel & 0x0F);
    
    // Word 2: Note# | Reserved | Pressure LSB | Pressure MSB...
    uint8_t noteB = note & 0x7F;
    uint32_t word2 = (static_cast<uint32_t>(noteB) << 24) | (pressure32 & 0x00FFFFFF);
    
    return (static_cast<uint64_t>(word1) << 32) | word2;
}

uint64_t UMPConverter::perNoteManagement(uint8_t note, uint8_t flags,
                                           uint8_t group, uint8_t channel)
{
    // Word 1: MT=3 | Group | Status=0xE (Per-Note Management) | Channel
    uint8_t status = (kMT_Midi2ChannelVoice << 4) | (group & 0x0F);
    uint8_t word1 = (status << 4) | (kStatus_PerNoteManagement << 0) | (channel & 0x0F);
    
    // Word 2: Note# | Flags
    uint8_t noteB = note & 0x7F;
    uint32_t word2 = (static_cast<uint32_t>(noteB) << 24) | 
                     (static_cast<uint32_t>(flags & 0x7F) << 16);
    
    return (static_cast<uint64_t>(word1) << 32) | word2;
}

// ==================== MIDI 1.0 Fallback Implementation ====================

uint32_t UMPConverter::noteOn1(uint8_t note, uint8_t velocity7, uint8_t channel)
{
    // Standard MIDI 1.0: Status | Note | Velocity
    uint8_t status = 0x90 | (channel & 0x0F);
    return (static_cast<uint32_t>(status) << 16) | 
           (static_cast<uint32_t>(note & 0x7F) << 8) | 
           static_cast<uint32_t>(velocity7 & 0x7F);
}

uint32_t UMPConverter::noteOff1(uint8_t note, uint8_t velocity7, uint8_t channel)
{
    uint8_t status = 0x80 | (channel & 0x0F);
    return (static_cast<uint32_t>(status) << 16) | 
           (static_cast<uint32_t>(note & 0x7F) << 8) | 
           static_cast<uint32_t>(velocity7 & 0x7F);
}

uint32_t UMPConverter::pitchBend1(uint8_t channel, uint16_t bend14)
{
    // MIDI 1.0 Pitch Bend: Status | LSB | MSB
    uint8_t status = 0xE0 | (channel & 0x0F);
    uint8_t lsb = static_cast<uint8_t>(bend14 & 0x7F);
    uint8_t msb = static_cast<uint8_t>((bend14 >> 7) & 0x7F);
    return (static_cast<uint32_t>(status) << 16) | 
           (static_cast<uint32_t>(lsb) << 8) | 
           static_cast<uint32_t>(msb);
}

uint32_t UMPConverter::channelAT1(uint8_t channel, uint8_t pressure7)
{
    uint8_t status = 0xD0 | (channel & 0x0F);
    return (static_cast<uint32_t>(status) << 8) | 
           static_cast<uint32_t>(pressure7 & 0x7F);
}

// ==================== Utility Implementation ====================

uint32_t UMPConverter::centsToPitchBend32(float cents)
{
    // Clamp to ±48 semitones (±4800 cents)
    cents = std::clamp(cents, -4800.0f, 4800.0f);
    
    // Normalize to 0-1 range
    float normalized = (cents + 4800.0f) / 9600.0f;
    
    // Scale to 32-bit unsigned range
    uint32_t result = static_cast<uint32_t>(normalized * 4294967295.0f);
    
    return result;
}

uint16_t UMPConverter::pitchBend32to14(uint32_t bend32)
{
    // Convert from 32-bit (0-4294967295, center 2147483648) 
    // to 14-bit (0-16383, center 8192)
    
    // First normalize to signed 32-bit (-2147483648 to +2147483647)
    int32_t signedBend = static_cast<int32_t>(bend32) - 2147483648;
    
    // Scale to 14-bit range
    // Division by 262144 = 2^28 / 2^14 = 16384
    int32_t scaled = signedBend / 262144;
    
    // Clamp and add offset
    scaled = std::clamp(scaled, -8192, 8191);
    
    return static_cast<uint16_t>(scaled + 8192);
}

uint8_t UMPConverter::velocity16to7(uint16_t vel16)
{
    // Convert 16-bit velocity to 7-bit
    // 65535 -> 127, 0 -> 0
    return static_cast<uint8_t>(vel16 >> 9);
}

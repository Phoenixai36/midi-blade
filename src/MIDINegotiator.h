#pragma once
#include <juce_core/juce_core.h>

// Protocol negotiation for MIDI 2.0 / MIDI 1.0 auto-detection
class MIDINegotiator
{
public:
    enum class ProtocolMode {
        Unknown,
        MIDI1_0,
        MIDI2_0
    };
    
    MIDINegotiator();
    ~MIDINegotiator();
    
    // Protocol detection
    ProtocolMode detectProtocol();
    void setProtocol(ProtocolMode mode);
    ProtocolMode getCurrentProtocol() const;
    bool isMIDI2Capable() const;
    
    // Auto-negotiation control
    void forceMIDI1();
    void forceMIDI2();
    void setAutoNegotiate(bool autoNeg);
    bool isAutoNegotiate() const;
    
    // MIDI-CI support (Phase B - stub for now)
    bool supportsMidiCI() const { return false; }
    
private:
    ProtocolMode currentMode;
    bool autoNegotiate;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MIDINegotiator)
};

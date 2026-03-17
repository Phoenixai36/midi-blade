#include "MIDINegotiator.h"

MIDINegotiator::MIDINegotiator()
    : currentMode(ProtocolMode::Unknown)
    , autoNegotiate(true)
{
}

MIDINegotiator::~MIDINegotiator() = default;

void MIDINegotiator::setProtocol(ProtocolMode mode)
{
    currentMode = mode;
}

MIDINegotiator::ProtocolMode MIDINegotiator::getCurrentProtocol() const
{
    return currentMode;
}

bool MIDINegotiator::isMIDI2Capable() const
{
    return currentMode == ProtocolMode::MIDI2_0;
}

void MIDINegotiator::forceMIDI1()
{
    autoNegotiate = false;
    currentMode = ProtocolMode::MIDI1_0;
}

void MIDINegotiator::forceMIDI2()
{
    autoNegotiate = false;
    currentMode = ProtocolMode::MIDI2_0;
}

void MIDINegotiator::setAutoNegotiate(bool autoNeg)
{
    autoNegotiate = autoNeg;
    if (autoNeg) {
        currentMode = ProtocolMode::Unknown;
    }
}

bool MIDINegotiator::isAutoNegotiate() const
{
    return autoNegotiate;
}

MIDINegotiator::ProtocolMode MIDINegotiator::detectProtocol()
{
    // TODO: Phase B - implement MIDI-CI discovery
    // For now, default to MIDI 2.0 if auto-negotiate
    if (currentMode == ProtocolMode::Unknown) {
        currentMode = ProtocolMode::MIDI2_0;  // Default to MIDI 2.0
    }
    return currentMode;
}

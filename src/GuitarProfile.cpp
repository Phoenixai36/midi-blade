#include "GuitarProfile.h"

// ============================================================
// GuitarProfile.cpp  –  Implementation
// ============================================================

GuitarProfile::GuitarProfile()
    : profileName("Guitar Tracker Profile")
    , maxFret(24)
    , profileEnabled(false)
{
    // Default to standard EADGBE tuning
    setStandardTuning();
}

GuitarProfile::~GuitarProfile() = default;

juce::String GuitarProfile::getProfileName() const
{
    return profileName;
}

juce::String GuitarProfile::getProfileVersion() const
{
    return juce::String(GuitarProfileConstants::kProfileVersionMajor) + "." +
           juce::String(GuitarProfileConstants::kProfileVersionMinor) + "." +
           juce::String(GuitarProfileConstants::kProfileVersionPatch);
}

std::array<uint8_t, 7> GuitarProfile::getProfileID() const
{
    std::array<uint8_t, 7> id{};
    for (int i = 0; i < 7; ++i) {
        id[i] = GuitarProfileConstants::kProfileID[i];
    }
    return id;
}

void GuitarProfile::setStandardTuning()
{
    // Standard EADGBE tuning (MIDI note numbers)
    currentTuning = { 40, 45, 50, 55, 59, 64 };  // E2, A2, D3, G3, B3, E4
}

void GuitarProfile::setDropDTuning()
{
    // Drop D tuning (DADGBE)
    currentTuning = { 38, 45, 50, 55, 59, 64 };  // D2, A2, D3, G3, B3, E4
}

void GuitarProfile::setOpenGTuning()
{
    // Open G tuning (DGDGBD)
    currentTuning = { 38, 43, 50, 55, 57, 62 };  // D2, G2, D3, G3, B3, D4
}

void GuitarProfile::setCustomTuning(const std::array<uint8_t, 6>& tuning)
{
    currentTuning = tuning;
}

void GuitarProfile::setFretRange(uint8_t maxFret_)
{
    maxFret = (maxFret_ > 24) ? 24 : maxFret_;
}

std::vector<uint8_t> GuitarProfile::buildProfileInquiry()
{
    // MIDI-CI Profile Inquiry message
    std::vector<uint8_t> msg;
    msg.push_back(0x7E);  // Non-realtime
    msg.push_back(0x10);  // MIDI-CI
    msg.push_back(0x02);  // Profile Inquiry
    // Add profile ID
    for (auto id : GuitarProfileConstants::kProfileID) {
        msg.push_back(id);
    }
    return msg;
}

std::vector<uint8_t> GuitarProfile::buildProfileEnabled()
{
    // MIDI-CI Profile Enabled message
    std::vector<uint8_t> msg;
    msg.push_back(0x7E);  // Non-realtime
    msg.push_back(0x10);  // MIDI-CI
    msg.push_back(0x05);  // Profile Enabled
    msg.push_back(0x00);  // Reserved
    // Add profile ID
    for (auto id : GuitarProfileConstants::kProfileID) {
        msg.push_back(id);
    }
    // Add profile version
    msg.push_back(GuitarProfileConstants::kProfileVersionMajor);
    msg.push_back(GuitarProfileConstants::kProfileVersionMinor);
    msg.push_back(GuitarProfileConstants::kProfileVersionPatch);
    
    profileEnabled = true;
    return msg;
}

std::vector<uint8_t> GuitarProfile::buildProfileDisabled()
{
    // MIDI-CI Profile Disabled message
    std::vector<uint8_t> msg;
    msg.push_back(0x7E);  // Non-realtime
    msg.push_back(0x10);  // MIDI-CI
    msg.push_back(0x06);  // Profile Disabled
    msg.push_back(0x00);  // Reserved
    // Add profile ID
    for (auto id : GuitarProfileConstants::kProfileID) {
        msg.push_back(id);
    }
    
    profileEnabled = false;
    return msg;
}

std::vector<uint8_t> GuitarProfile::buildProfileCapabilityInquiry()
{
    // MIDI-CI Profile Capability Inquiry message
    std::vector<uint8_t> msg;
    msg.push_back(0x7E);  // Non-realtime
    msg.push_back(0x10);  // MIDI-CI
    msg.push_back(0x03);  // Capability Inquiry
    msg.push_back(0x00);  // Reserved
    // Add profile ID
    for (auto id : GuitarProfileConstants::kProfileID) {
        msg.push_back(id);
    }
    
    // Add capabilities (bitfield)
    msg.push_back(0xFF);  // Capabilities byte 1: All features supported
    msg.push_back(0x03);  // Capabilities byte 2: 
    msg.push_back(0x00);  // Reserved
    msg.push_back(0x00);  // Reserved
    
    return msg;
}

void GuitarProfile::processMIDI_CI(const uint8_t* data, size_t length)
{
    if (length < 4) return;
    
    // Handle incoming MIDI-CI messages
    // This is a basic implementation - more sophisticated parsing would go here
    juce::ignoreUnused(data, length);
}

void GuitarProfile::setProperty(uint16_t propertyID, const std::vector<uint8_t>& value)
{
    properties[propertyID] = value;
}

std::vector<uint8_t> GuitarProfile::getProperty(uint16_t propertyID) const
{
    auto it = properties.find(propertyID);
    if (it != properties.end()) {
        return it->second;
    }
    return {};
}

bool GuitarProfile::hasProperty(uint16_t propertyID) const
{
    return properties.find(propertyID) != properties.end();
}

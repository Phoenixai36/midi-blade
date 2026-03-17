#pragma once
#include <juce_core/juce_core.h>
#include <array>
#include <vector>
#include <map>
#include <functional>
#include <cstdint>

// ============================================================
// GuitarProfile.h  –  Custom MIDI-CI Profile for Guitar Tracking
// Implements MIDI-CI Profile for polyphonic guitar-to-MIDI conversion
// ============================================================

// Guitar Tracker Profile Identity
namespace GuitarProfileConstants {
    // Manufacturer ID (Phoenixai36 - placeholder)
    static constexpr uint8_t kManufacturerID[3] = { 0x00, 0x01, 0x36 };
    
    // Profile ID for Guitar Tracker
    static constexpr uint8_t kProfileID[7] = { 
        0x47,  // 'G' - Guitar
        0x54,  // 'T' - Tracker  
        0x52,  // 'R' - Profile
        0x31,  // '1' - Version 1
        0x00, 0x00, 0x00  // Reserved
    };
    
    // Profile Version: 1.0.0
    static constexpr uint8_t kProfileVersionMajor = 1;
    static constexpr uint8_t kProfileVersionMinor = 0;
    static constexpr uint8_t kProfileVersionPatch = 0;
    
    // Device ID for this profile
    static constexpr uint8_t kDeviceType = 0x01;  // Guitar-to-MIDI converter
    
    // Sub-device IDs
    static constexpr uint8_t kSubDevice_String1 = 0x01;
    static constexpr uint8_t kSubDevice_String2 = 0x02;
    static constexpr uint8_t kSubDevice_String3 = 0x03;
    static constexpr uint8_t kSubDevice_String4 = 0x04;
    static constexpr uint8_t kSubDevice_String5 = 0x05;
    static constexpr uint8_t kSubDevice_String6 = 0x06;
}

// Property definitions for guitar-specific data
namespace GuitarPropertyIDs {
    // Standard MIDI-CI properties
    static constexpr uint16_t kProp_Velocity          = 0x0001;
    static constexpr uint16_t kProp_PitchBend         = 0x0002;
    static constexpr uint16_t kProp_Aftertouch        = 0x0003;
    
    // Guitar-specific properties (Manufacturer-specific range)
    static constexpr uint16_t kProp_StringNumber       = 0x1000;  // Which string triggered
    static constexpr uint16_t kProp_FretNumber        = 0x1001;  // Fret position (0-24)
    static constexpr uint16_t kProp_PickDirection      = 0x1002;  // Up/down pick direction
    static constexpr uint16_t kProp_PickVelocity      = 0x1003;  // Picking intensity
    static constexpr uint16_t kProp_MuteStatus        = 0x1004;  // Palm mute, etc.
    static constexpr uint16_t kProp_SlideIndicator    = 0x1005;  // Slide in progress
    static constexpr uint16_t kProp_HammerOn          = 0x1006;  // Hammer-on detected
    static constexpr uint16_t kProp_PullOff           = 0x1007;  // Pull-off detected
    static constexpr uint16_t kProp_BendAmount        = 0x1008;  // Bend amount in cents
    static constexpr uint16_t kProp_VibratoDepth      = 0x1009;  // Vibrato intensity
    static constexpr uint16_t kProp_StringMute        = 0x100A;  // Individual string mute
    static constexpr uint16_t kProp_HarmonicType      = 0x100B;  // Natural/artificial harmonic
    static constexpr uint16_t kProp_PitchConfidence   = 0x100C;  // Detection confidence
    static constexpr uint16_t kProp_Timestamp        = 0x100D;  // Sample-accurate timestamp
}

// Guitar-specific MIDI-CI Message Types
enum class GuitarMessageType : uint8_t {
    NoteOn                    = 0x01,
    NoteOff                   = 0x02,
    StringTrigger             = 0x10,
    FretPosition              = 0x11,
    PickDirection             = 0x12,
    StringMute                = 0x13,
    TechniqueEvent            = 0x14,  // Hammer, pull, slide
    PitchBendHighRes          = 0x15,
    VelocityHighRes           = 0x16,
    BendCurve                 = 0x17,  // Custom bend curves
    Harmonic                  = 0x18,
    TuningChange              = 0x20,
    CapoPosition              = 0x21,
    TuningOffset              = 0x22,
    StringConfig              = 0x30,  // 6-string, 7-string, etc.
    SensorCalibration         = 0x40,
    ModelLoad                 = 0x50,
};

// Pick direction enumeration
enum class PickDirection : uint8_t {
    Unknown   = 0,
    Up        = 1,
    Down      = 2,
    Rest      = 3,  // Palm mute/rest stroke
};

// Guitar techniques
enum class GuitarTechnique : uint8_t {
    None           = 0,
    HammerOn       = 1,
    PullOff        = 2,
    Slide          = 3,
    Bend           = 4,
    Vibrato        = 5,
    Harmonic       = 6,
    PalmMute       = 7,
    Tapping        = 8,
    Slap           = 9,
    Pop            = 10,
    Tremolo        = 11,
};

// Fret position data (0-24 frets for standard guitar)
struct FretPosition {
    uint8_t fretNumber;     // 0-24 (0 = open string)
    uint8_t stringNumber;   // 1-6
    uint8_t confidence;     // 0-100 detection confidence
    
    FretPosition() : fretNumber(0), stringNumber(0), confidence(0) {}
    FretPosition(uint8_t fret, uint8_t string, uint8_t conf) 
        : fretNumber(fret), stringNumber(string), confidence(conf) {}
};

// High-resolution pitch bend for guitar
struct GuitarPitchBend {
    uint8_t stringNumber;      // 1-6
    int32_t bendCents;         // -4800 to +4800 cents (±2 octaves for guitar)
    uint8_t bendType;         // 0=regular, 1=smooth, 2=stepped
    uint8_t confidence;       // 0-100
    
    GuitarPitchBend() : stringNumber(0), bendCents(0), bendType(0), confidence(0) {}
};

// Guitar Tracker Profile class
class GuitarProfile {
public:
    GuitarProfile();
    ~GuitarProfile();
    
    // Profile information
    juce::String getProfileName() const;
    juce::String getProfileVersion() const;
    std::array<uint8_t, 7> getProfileID() const;
    
    // Profile capabilities
    bool supportsHighResVelocity() const { return true; }
    bool supportsHighResPitchBend() const { return true; }
    bool supportsStringDetection() const { return true; }
    bool supportsFretDetection() const { return true; }
    bool supportsTechniqueDetection() const { return true; }
    
    // Number of strings supported
    int getMaxStrings() const { return GuitarProfileConstants::kSubDevice_String6; }
    
    // Tuning configuration
    void setStandardTuning();        // EADGBE
    void setDropDTuning();          // DADGBE
    void setOpenGTuning();           // DGDGBD
    void setCustomTuning(const std::array<uint8_t, 6>& tuning);
    
    std::array<uint8_t, 6> getCurrentTuning() const { return currentTuning; }
    
    // Fret range
    void setFretRange(uint8_t maxFret);
    uint8_t getMaxFret() const { return maxFret; }
    
    // MIDI-CI Profile messages
    std::vector<uint8_t> buildProfileInquiry();
    std::vector<uint8_t> buildProfileEnabled();
    std::vector<uint8_t> buildProfileDisabled();
    std::vector<uint8_t> buildProfileCapabilityInquiry();
    
    // Process incoming MIDI-CI messages
    void processMIDI_CI(const uint8_t* data, size_t length);
    
    // Property access
    void setProperty(uint16_t propertyID, const std::vector<uint8_t>& value);
    std::vector<uint8_t> getProperty(uint16_t propertyID) const;
    bool hasProperty(uint16_t propertyID) const;
    
    // Notification callbacks (for external handling)
    std::function<void(FretPosition)> onFretDetected;
    std::function<void(GuitarTechnique)> onTechniqueDetected;
    std::function<void(GuitarPitchBend)> onPitchBendDetected;
    
private:
    juce::String profileName;
    std::array<uint8_t, 6> currentTuning;
    uint8_t maxFret;
    bool profileEnabled;
    
    std::map<uint16_t, std::vector<uint8_t>> properties;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GuitarProfile)
};

#pragma once
#include <juce_dsp/juce_dsp.h>
#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <vector>

// ============================================================
// BladeDSP.h  –  Hex-blade IIR filterbank + crepe stub
// 6 per-string bandpass filters (EADGBE)
// crepe_guitar.ptl loaded via LibTorch when REBORN_USE_TORCH=1
// ============================================================

// Configuration constants
static constexpr int MAX_STRINGS = 6;     // 6-string guitar support
static constexpr int MAX_DEVICES = 8;     // Multiple device support

#ifdef REBORN_USE_TORCH
  #include <torch/script.h>
  #include <torch/torch.h>
static constexpr bool kUseTorch = true;
#else
static constexpr bool kUseTorch = false;
#endif

#include "MIDINegotiator.h"
#include "UMPConverter.h"

struct NoteEvent {
    int    note;      // MIDI note 0-127
    int    velocity;  // 0-127 (MIDI 1.0) or 0-65535 (MIDI 2.0)
    int    samplePos; // position in block
    bool   isOn;
    
    // 16-bit velocity support
    uint16_t velocity16 = 0;  // Full 16-bit velocity for MIDI 2.0
};

// Velocity utility functions for 16-bit resolution
namespace VelocityUtils {
    // Convert normalized float (0.0-1.0) to 16-bit velocity
    static constexpr uint16_t floatToVelocity16(float normalized) {
        return static_cast<uint16_t>(std::clamp(normalized * 65535.0f, 0.0f, 65535.0f));
    }
    
    // Convert 16-bit velocity to 7-bit (MIDI 1.0 fallback)
    static constexpr uint8_t velocity16to7(uint16_t vel16) {
        return static_cast<uint8_t>(vel16 >> 9);  // Divide by 512
    }
    
    // Convert 16-bit velocity to 14-bit (MIDI 1.0 pitch bend compatible)
    static constexpr uint16_t velocity16to14(uint16_t vel16) {
        return static_cast<uint16_t>(vel16 >> 2);  // Divide by 4
    }
    
    // Apply velocity curve (linear, exponential, or custom)
    static uint16_t applyVelocityCurve(float normalized, int curveType = 0) {
        float curved = normalized;
        switch (curveType) {
            case 1:  // Exponential curve
                curved = normalized * normalized;
                break;
            case 2:  // Logarithmic curve
                curved = std::sqrt(normalized);
                break;
            default: // Linear
                break;
        }
        return floatToVelocity16(curved);
    }
}

// Per-string note state for tracking
struct BladeState {
    uint8_t  noteNumber = 0;       // MIDI note (0-127)
    uint8_t  channel = 0;           // MIDI channel
    uint8_t  group = 0;            // UMP group
    
    uint16_t velocity16 = 0;       // 16-bit velocity
    uint8_t  velocity7 = 0;        // 7-bit velocity (fallback)
    
    uint32_t pitchBend32 = 0x80000000;  // 32-bit pitch (center)
    uint32_t aftertouch32 = 0;     // 32-bit per-note AT
    
    bool     active = false;
    bool     noteOnSent = false;
    
    float    f0Hz = 0.0f;
    float    lastF0Hz = 0.0f;
    int64_t  pitchAccumulator = 0;
    
    uint32_t noteOnTime = 0;
    uint32_t lastUpdateTime = 0;
};

class BladeDSP {
public:
    // Fundamental freqs EADGBE (Hz)
    static constexpr double kStringFreqs[6] = {82.4, 110.0, 146.8, 196.0, 246.9, 329.6};
    static constexpr double kQ             = 4.0;    // bandpass Q
    static constexpr float  kNoiseFloor    = 0.01f;  // gate threshold

    BladeDSP()  = default;
    ~BladeDSP() = default;

    void prepare(double sampleRate, int blockSize);
    void loadCrepeModel(const juce::String& ptlPath);
    std::vector<NoteEvent> process(const float* monoAudio, int numSamples);
    void reset();

    // Threshold for note-on (0.0–1.0)
    float threshold { 0.3f };

    // Protocol access
    MIDINegotiator& getMidiNegotiator() { return midiNegotiator; }

    // Per-blade state access
    BladeState& getBladeState(int stringIndex);

    // Process with MIDI 2.0
    void processMIDI2(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
    void processMIDI1(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);

private:
    double sr { 44100.0 };
    std::array<juce::dsp::IIR::Filter<float>, 6> blades;
    std::array<float, 6> envFollower {};
    std::array<bool,  6> activeNote  {};
    std::array<int,   6> midiNote    {};

    // MIDI 2.0 support
    MIDINegotiator midiNegotiator;
    UMPConverter umpConverter;
    BladeState bladeStates[6];  // 6 strings

    int frequencyToMidiNote(float hz) const;
    float detectVelocity(int stringIndex) const;
    bool detectNoteOff(int stringIndex) const;

#ifdef REBORN_USE_TORCH
    torch::jit::script::Module crepeModel;
    bool modelLoaded { false };
    std::vector<NoteEvent> crepePredict(const float* audio, int numSamples);
#endif

    std::vector<NoteEvent> bladeTrack(const float* audio, int numSamples);
    float hzToVelocity(float rms) const;
};

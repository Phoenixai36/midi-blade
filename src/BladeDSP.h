#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <vector>

// ============================================================
// BladeDSP.h  –  Hex-blade IIR filterbank + crepe stub
// 6 per-string bandpass filters (EADGBE)
// crepe_guitar.ptl loaded via LibTorch when REBORN_USE_TORCH=1
// ============================================================

#ifdef REBORN_USE_TORCH
  #include <torch/script.h>
  #include <torch/torch.h>
static constexpr bool kUseTorch = true;
#else
static constexpr bool kUseTorch = false;
#endif

struct NoteEvent {
    int    note;      // MIDI note 0-127
    int    velocity;  // 0-127
    int    samplePos; // position in block
    bool   isOn;
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

private:
    double sr { 44100.0 };
    std::array<juce::dsp::IIR::Filter<float>, 6> blades;
    std::array<float, 6> envFollower {};
    std::array<bool,  6> activeNote  {};
    std::array<int,   6> midiNote    {};

#ifdef REBORN_USE_TORCH
    torch::jit::script::Module crepeModel;
    bool modelLoaded { false };
    std::vector<NoteEvent> crepePredict(const float* audio, int numSamples);
#endif

    std::vector<NoteEvent> bladeTrack(const float* audio, int numSamples);
    float hzToVelocity(float rms) const;
};

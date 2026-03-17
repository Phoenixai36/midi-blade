#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "UMPConverter.h"

class BladeProcessor : public juce::AudioProcessor
{
public:
    BladeProcessor();
    ~BladeProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void releaseResources() override;

    const juce::String getName() const override { return "MIDI Blade"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return true; }

private:
    // 6 blades: E2 A2 D3 G3 B3 E4 (bandpass IIR per string)
    static constexpr int NUM_BLADES = 6;
    const float STRING_FREQS[NUM_BLADES] = { 82.41f, 110.0f, 146.83f, 196.0f, 246.94f, 329.63f };

    std::array<juce::dsp::IIR::Filter<float>, NUM_BLADES> bladeFilters;
    UMPConverter umpConverter;
    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BladeProcessor)
};

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "BladeDSP.h"
#include "UMPConverter.h"

class BladeProcessor : public juce::AudioProcessor
{
public:
    BladeProcessor();
    ~BladeProcessor() override;

    // ---- Playback ----
    void prepareToPlay  (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock   (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // ---- Identity ----
    const juce::String getName() const override { return "RebornGuitar"; }
    bool acceptsMidi()   const override { return false; }
    bool producesMidi()  const override { return true;  }
    bool isMidiEffect()  const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    // ---- Editor (headless / no GUI) ----
    bool hasEditor() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }

    // ---- Programs ----
    int  getNumPrograms()                                         override { return 1; }
    int  getCurrentProgram()                                      override { return 0; }
    void setCurrentProgram (int)                                  override {}
    const juce::String getProgramName (int)                       override { return "Default"; }
    void changeProgramName (int, const juce::String&)             override {}

    // ---- State ----
    void getStateInformation (juce::MemoryBlock& destData)        override;
    void setStateInformation (const void* data, int sizeInBytes)  override;

private:
    BladeDSP   bladeDSP;
    UMPConverter umpConverter;
    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BladeProcessor)
};

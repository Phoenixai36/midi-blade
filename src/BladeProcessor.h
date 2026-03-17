#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "BladeDSP.h"
#include "UMPConverter.h"
#include "RebornParameters.h"

class RebornEditor;

class BladeProcessor : public juce::AudioProcessor,
                       public juce::AudioProcessorValueTreeState::Listener
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

    // ---- Editor ----
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;

    // ---- Programs ----
    int  getNumPrograms()                              override { return 1; }
    int  getCurrentProgram()                           override { return 0; }
    void setCurrentProgram(int)                        override {}
    const juce::String getProgramName(int)             override { return "Default"; }
    void changeProgramName(int, const juce::String&)   override {}

    // ---- State ----
    void getStateInformation(juce::MemoryBlock& dest)          override;
    void setStateInformation(const void* data, int bytes)      override;

    // ---- Parameter listener ----
    void parameterChanged(const juce::String& paramID, float newValue) override;

    // ---- Public API for MCP/REST bridge ----
    void setThreshold(float v)   { bladeDSP.threshold = v; }
    void setMidiChannel(int ch)  { midiChannel = ch; }
    float getThreshold() const   { return bladeDSP.threshold; }
    int   getMidiChannel() const { return midiChannel; }
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

private:
    BladeDSP    bladeDSP;
    UMPConverter umpConverter;
    juce::AudioProcessorValueTreeState apvts;
    int midiChannel { 1 };
    double currentSampleRate { 44100.0 };

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BladeProcessor)
};

#include "BladeProcessor.h"

BladeProcessor::BladeProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::mono(), true)
        .withOutput("Output", juce::AudioChannelSet::mono(), true))
{}

BladeProcessor::~BladeProcessor() {}

void BladeProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec{ sampleRate, (juce::uint32)samplesPerBlock, 1 };

    for (int i = 0; i < NUM_BLADES; ++i)
    {
        // Bandpass filter centred on each string frequency
        auto coeffs = juce::dsp::IIR::Coefficients<float>::makeBandPass(
            sampleRate, STRING_FREQS[i], 2.0f);
        *bladeFilters[i].coefficients = *coeffs;
        bladeFilters[i].prepare(spec);
    }
}

void BladeProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // TODO: Feed each blade filter → crepe f0 detect → UMP out
    // Phase 3 implementation
    for (int blade = 0; blade < NUM_BLADES; ++blade)
    {
        auto channelData = buffer.getWritePointer(0);
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        bladeFilters[blade].process(context);

        // TODO: crepe f0 → umpConverter.convert(f0, blade) → midiMessages
    }
}

void BladeProcessor::releaseResources() {}

#include "BladeProcessor.h"
#include "BladeDSP.h"

// ============================================================
// JUCE entry point – required by VST3/Standalone plugin client
// ============================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BladeProcessor();
}

// ============================================================
// BladeProcessor implementation
// ============================================================

BladeProcessor::BladeProcessor()
    : AudioProcessor(BusesProperties()
        .withInput ("Input",  juce::AudioChannelSet::mono(), true)
        .withOutput("Output", juce::AudioChannelSet::mono(), true))
{}

BladeProcessor::~BladeProcessor() {}

void BladeProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    bladeDSP.prepare(sampleRate, samplesPerBlock);
}

void BladeProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                  juce::MidiBuffer&         midiMessages)
{
    juce::ignoreUnused(midiMessages);

    if (buffer.getNumChannels() < 1 || buffer.getNumSamples() < 1)
        return;

    const float* monoIn  = buffer.getReadPointer(0);
    const int    numSamp = buffer.getNumSamples();

    // Run hex-blade IIR tracker (or crepe if REBORN_USE_TORCH compiled)
    auto events = bladeDSP.process(monoIn, numSamp);

    // Push NoteEvents into MIDI buffer for downstream instruments
    for (const auto& ev : events)
    {
        auto msg = ev.isOn
            ? juce::MidiMessage::noteOn (1, ev.note, (juce::uint8)ev.velocity)
            : juce::MidiMessage::noteOff(1, ev.note);
        midiMessages.addEvent(msg, ev.samplePos);
    }

    // Clear audio out (this is a MIDI-only plugin)
    buffer.clear();
}

void BladeProcessor::releaseResources()
{
    bladeDSP.reset();
}

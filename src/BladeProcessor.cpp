#include "BladeProcessor.h"

// ---- JUCE entry point ------------------------------------------------
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BladeProcessor();
}

// ---- Constructor / Destructor ----------------------------------------
BladeProcessor::BladeProcessor()
    : AudioProcessor(BusesProperties()
        .withInput ("Input",  juce::AudioChannelSet::mono(), true)
        .withOutput("Output", juce::AudioChannelSet::mono(), true))
{}

BladeProcessor::~BladeProcessor() {}

// ---- Playback --------------------------------------------------------
void BladeProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    bladeDSP.prepare(sampleRate, samplesPerBlock);
}

void BladeProcessor::releaseResources()
{
    bladeDSP.reset();
}

void BladeProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                  juce::MidiBuffer&         midiMessages)
{
    if (buffer.getNumChannels() < 1 || buffer.getNumSamples() < 1)
        return;

    // Auto-detect MIDI protocol on first run
    static bool protocolInitialized = false;
    if (!protocolInitialized) {
        bladeDSP.getMidiNegotiator().detectProtocol();
        protocolInitialized = true;
    }
    
    // Check current protocol mode
    auto protocol = bladeDSP.getMidiNegotiator().getCurrentProtocol();
    
    // Get audio data
    const float* monoIn  = buffer.getReadPointer(0);
    const int    numSamp = buffer.getNumSamples();

    if (protocol == MIDINegotiator::ProtocolMode::MIDI2_0) {
        // Use MIDI 2.0 processing with 32-bit per-note expression
        bladeDSP.processMIDI2(buffer, midiMessages);
    } else {
        // Fall back to MIDI 1.0
        bladeDSP.processMIDI1(buffer, midiMessages);
    }

    // Also run the standard blade tracker for legacy compatibility
    std::vector<NoteEvent> events = bladeDSP.process(monoIn, numSamp);

    for (size_t i = 0; i < events.size(); ++i)
    {
        const NoteEvent& ev = events[i];
        juce::MidiMessage msg;
        if (ev.isOn)
            msg = juce::MidiMessage::noteOn (1, ev.note, (juce::uint8)ev.velocity);
        else
            msg = juce::MidiMessage::noteOff(1, ev.note, (juce::uint8)0);

        midiMessages.addEvent(msg, ev.samplePos);
    }

    buffer.clear();  // MIDI-only plugin, no audio out
}

// ---- State -----------------------------------------------------------
void BladeProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::ignoreUnused(destData);  // no params yet
}

void BladeProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::ignoreUnused(data, sizeInBytes);
}

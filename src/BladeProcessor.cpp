#include "BladeProcessor.h"
#include "RebornEditor.h"

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new BladeProcessor(); }

// ---- Parameter layout ------------------------------------------------
juce::AudioProcessorValueTreeState::ParameterLayout BladeProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"threshold", 1}, "Threshold",
        juce::NormalisableRange<float>(0.01f, 1.0f, 0.01f), 0.30f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"sensitivity", 1}, "Sensitivity",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.75f));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{"midi_ch", 1}, "MIDI Channel", 1, 16, 1));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"poly_mode", 1}, "Polyphonic", true));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{"root_note", 1}, "Root Note", 0, 11, 0));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{"scale", 1}, "Scale", 0, 7, 0));

    return { params.begin(), params.end() };
}

// ---- Constructor -----------------------------------------------------
BladeProcessor::BladeProcessor()
    : AudioProcessor(BusesProperties()
        .withInput ("Input",  juce::AudioChannelSet::mono(), true)
        .withOutput("Output", juce::AudioChannelSet::mono(), true)),
      apvts(*this, nullptr, "RebornGuitar", createParameterLayout())
{
    apvts.addParameterListener("threshold",   this);
    apvts.addParameterListener("sensitivity", this);
    apvts.addParameterListener("midi_ch",     this);
    apvts.addParameterListener("poly_mode",   this);
}

BladeProcessor::~BladeProcessor()
{
    apvts.removeParameterListener("threshold",   this);
    apvts.removeParameterListener("sensitivity", this);
    apvts.removeParameterListener("midi_ch",     this);
    apvts.removeParameterListener("poly_mode",   this);
}

// ---- Playback --------------------------------------------------------
void BladeProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    bladeDSP.prepare(sampleRate, samplesPerBlock);
}

void BladeProcessor::releaseResources() { bladeDSP.reset(); }

void BladeProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                  juce::MidiBuffer& midiMessages)
{
    if (buffer.getNumChannels() < 1 || buffer.getNumSamples() < 1) return;

    std::vector<NoteEvent> events = bladeDSP.process(
        buffer.getReadPointer(0), buffer.getNumSamples());

    for (size_t i = 0; i < events.size(); ++i) {
        const NoteEvent& ev = events[i];
        juce::MidiMessage msg = ev.isOn
            ? juce::MidiMessage::noteOn (midiChannel, ev.note, (juce::uint8)ev.velocity)
            : juce::MidiMessage::noteOff(midiChannel, ev.note, (juce::uint8)0);
        midiMessages.addEvent(msg, ev.samplePos);
    }
    buffer.clear();
}

// ---- Editor ----------------------------------------------------------
juce::AudioProcessorEditor* BladeProcessor::createEditor()
{
    return new RebornEditor(*this, apvts);
}

// ---- State -----------------------------------------------------------
void BladeProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}

void BladeProcessor::setStateInformation(const void* data, int bytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, bytes));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

// ---- Parameter listener -----------------------------------------------
void BladeProcessor::parameterChanged(const juce::String& id, float v)
{
    if (id == "threshold")   bladeDSP.threshold = v;
    if (id == "sensitivity") bladeDSP.threshold = v * 0.5f;
    if (id == "midi_ch")     midiChannel = (int)v;
}

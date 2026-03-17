#include "BladeDSP.h"
#include <cmath>
#include <algorithm>

// ============================================================
// BladeDSP.cpp  –  Implementation
// ============================================================

void BladeDSP::prepare(double sampleRate, int /*blockSize*/)
{
    sr = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, 512, 1 };

    for (int i = 0; i < 6; ++i) {
        // Bandpass centred on each open-string fundamental
        *blades[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeBandPass(
            sampleRate, kStringFreqs[i], kQ);
        blades[i].prepare(spec);
        blades[i].reset();
        envFollower[i] = 0.f;
        activeNote[i]  = false;
        // Map string idx → MIDI root note (E2=40, A2=45, D3=50, G3=55, B3=59, E4=64)
        static const int roots[6] = { 40, 45, 50, 55, 59, 64 };
        midiNote[i] = roots[i];
    }
}

void BladeDSP::reset()
{
    for (auto& f : blades) f.reset();
    envFollower.fill(0.f);
    activeNote.fill(false);
}

void BladeDSP::loadCrepeModel(const juce::String& ptlPath)
{
#ifdef REBORN_USE_TORCH
    try {
        crepeModel  = torch::jit::load(ptlPath.toStdString());
        crepeModel.eval();
        modelLoaded = true;
    } catch (const c10::Error& e) {
        juce::Logger::writeToLog("[BladeDSP] crepe model load failed: " + juce::String(e.what()));
        modelLoaded = false;
    }
#else
    juce::ignoreUnused(ptlPath);
    juce::Logger::writeToLog("[BladeDSP] LibTorch not compiled in – using IIR blade tracking");
#endif
}

std::vector<NoteEvent> BladeDSP::process(const float* monoAudio, int numSamples)
{
#ifdef REBORN_USE_TORCH
    if (modelLoaded)
        return crepePredict(monoAudio, numSamples);
#endif
    return bladeTrack(monoAudio, numSamples);
}

// ---- IIR blade tracker (no-torch fallback) ----
std::vector<NoteEvent> BladeDSP::bladeTrack(const float* audio, int numSamples)
{
    std::vector<NoteEvent> events;
    const float attackCoeff  = std::exp(-1.f / (0.003f * (float)sr));  // 3ms attack
    const float releaseCoeff = std::exp(-1.f / (0.08f  * (float)sr));  // 80ms release

    // Run each blade filter over the full block and track envelope
    for (int s = 0; s < numSamples; ++s) {
        float sample = audio[s];

        for (int i = 0; i < 6; ++i) {
            float filtered = blades[i].processSample(sample);
            float rectified = std::abs(filtered);

            float& env = envFollower[i];
            env = (rectified > env)
                ? attackCoeff  * env + (1.f - attackCoeff)  * rectified
                : releaseCoeff * env + (1.f - releaseCoeff) * rectified;

            bool shouldBeOn = env > threshold;

            if (shouldBeOn && !activeNote[i]) {
                activeNote[i] = true;
                events.push_back({ midiNote[i], (int)(hzToVelocity(env) * 127.f), s, true });
            } else if (!shouldBeOn && activeNote[i]) {
                activeNote[i] = false;
                events.push_back({ midiNote[i], 0, s, false });
            }
        }
    }
    return events;
}

float BladeDSP::hzToVelocity(float rms) const
{
    return std::clamp(rms * 4.f, 0.f, 1.f);
}

// ---- MIDI 2.0 Helper Methods ----

int BladeDSP::frequencyToMidiNote(float hz) const
{
    if (hz <= 0) return 0;
    return static_cast<int>(std::round(69.0f + 12.0f * std::log2(hz / 440.0f)));
}

float BladeDSP::detectVelocity(int stringIndex) const
{
    return hzToVelocity(envFollower[stringIndex]);
}

bool BladeDSP::detectNoteOff(int stringIndex) const
{
    return envFollower[stringIndex] < threshold;
}

BladeState& BladeDSP::getBladeState(int stringIndex)
{
    return bladeStates[stringIndex];
}

// ---- MIDI 2.0 Processing ----

void BladeDSP::processMIDI2(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // For each active blade/string
    for (int i = 0; i < 6; ++i) {
        auto& blade = bladeStates[i];
        
        // Get detected pitch from filters
        // For IIR mode, use string fundamental; for CREPE, use detected pitch
        float f0 = 0.0f;
        if (envFollower[i] > threshold) {
            f0 = static_cast<float>(kStringFreqs[i]);
        }
        
        if (f0 > 0 && !blade.active) {
            // Note On detected
            blade.active = true;
            blade.f0Hz = f0;
            blade.lastF0Hz = f0;
            blade.noteNumber = static_cast<uint8_t>(frequencyToMidiNote(f0));
            blade.velocity16 = static_cast<uint16_t>(detectVelocity(i) * 65535.0f);
            blade.velocity7 = static_cast<uint8_t>(blade.velocity16 >> 9);
            blade.pitchBend32 = 0x80000000;  // Reset to center
            
            // Generate MIDI 2.0 Note On
            uint64_t ump = umpConverter.noteOn32(
                blade.noteNumber, 
                blade.velocity16,
                blade.group, 
                blade.channel
            );
            // Add to MIDI buffer (needs proper UMP handling)
            midiMessages.addEvent(static_cast<int>(ump), 0);  // Placeholder
            blade.noteOnSent = true;
            
        } else if (blade.active) {
            // Track pitch changes for pitch bend
            // In real implementation, this would come from pitch detection
            float pitchDelta = f0 - blade.lastF0Hz;
            
            if (std::abs(pitchDelta) > 0.1f) {
                // Convert frequency delta to cents
                float cents = 1200.0f * std::log2(f0 / blade.lastF0Hz);
                
                // Convert to 32-bit pitch bend
                blade.pitchBend32 = UMPConverter::centsToPitchBend32(cents);
                
                // Generate MIDI 2.0 Per-Note Pitch Bend
                uint64_t ump = umpConverter.pitchBend32(
                    blade.noteNumber,
                    blade.pitchBend32,
                    blade.group,
                    blade.channel
                );
                midiMessages.addEvent(static_cast<int>(ump), 0);  // Placeholder
                
                blade.lastF0Hz = f0;
            }
            
            // Check for note off (no signal)
            if (f0 <= 0 || detectNoteOff(i)) {
                blade.active = false;
                
                // Generate MIDI 2.0 Note Off
                uint64_t ump = umpConverter.noteOff32(
                    blade.noteNumber,
                    0,  // Release velocity
                    blade.group,
                    blade.channel
                );
                midiMessages.addEvent(static_cast<int>(ump), 0);  // Placeholder
                
                blade.noteOnSent = false;
            }
        }
    }
}

void BladeDSP::processMIDI1(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // MIDI 1.0 fallback - same logic but with 14-bit/7-bit values
    for (int i = 0; i < 6; ++i) {
        auto& blade = bladeStates[i];
        float f0 = 0.0f;
        if (envFollower[i] > threshold) {
            f0 = static_cast<float>(kStringFreqs[i]);
        }
        
        if (f0 > 0 && !blade.active) {
            blade.active = true;
            blade.noteNumber = static_cast<uint8_t>(frequencyToMidiNote(f0));
            blade.velocity7 = static_cast<uint8_t>(detectVelocity(i) * 127.0f);
            
            // MIDI 1.0 Note On
            uint32_t msg = umpConverter.noteOn1(
                blade.noteNumber,
                blade.velocity7,
                blade.channel
            );
            // Add to MIDI buffer
            midiMessages.addEvent(static_cast<int>(msg), 0);
            
        } else if (blade.active) {
            float pitchDelta = f0 - blade.lastF0Hz;
            
            if (std::abs(pitchDelta) > 0.1f) {
                float cents = 1200.0f * std::log2(f0 / blade.lastF0Hz);
                uint32_t bend32 = UMPConverter::centsToPitchBend32(cents);
                uint16_t bend14 = UMPConverter::pitchBend32to14(bend32);
                
                uint32_t msg = umpConverter.pitchBend1(blade.channel, bend14);
                midiMessages.addEvent(static_cast<int>(msg), 0);
                
                blade.lastF0Hz = f0;
            }
            
            if (f0 <= 0 || detectNoteOff(i)) {
                blade.active = false;
                uint32_t msg = umpConverter.noteOff1(blade.noteNumber, 0, blade.channel);
                midiMessages.addEvent(static_cast<int>(msg), 0);
            }
        }
    }
}

// ---- LibTorch crepe predictor ----
#ifdef REBORN_USE_TORCH
std::vector<NoteEvent> BladeDSP::crepePredict(const float* audio, int numSamples)
{
    std::vector<NoteEvent> events;
    auto tensor = torch::from_blob(
        const_cast<float*>(audio),
        { 1, numSamples },
        torch::kFloat32
    ).clone();

    torch::NoGradGuard no_grad;
    auto output = crepeModel.forward({ tensor }).toTensor();  // [N, 3] = [pitch, vel, isOn]

    for (int i = 0; i < output.size(0); ++i) {
        float pitch = output[i][0].item<float>();
        float vel   = output[i][1].item<float>();
        bool  on    = output[i][2].item<float>() > 0.5f;

        int midiPitch = (int)std::round(69.f + 12.f * std::log2(pitch / 440.f));
        midiPitch = std::clamp(midiPitch, 0, 127);

        events.push_back({ midiPitch, (int)(vel * 127.f), i, on });
    }
    return events;
}
#endif

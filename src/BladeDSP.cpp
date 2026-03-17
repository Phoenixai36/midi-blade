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

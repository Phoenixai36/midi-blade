#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class BladeProcessor;

// ============================================================
// RebornEditor  — Main plugin GUI (VST3 + Standalone identical)
// Design: Dark cyberpunk, 6 blade VU meters, parameter panel
// ============================================================
class RebornEditor  : public juce::AudioProcessorEditor,
                      private juce::Timer
{
public:
    RebornEditor(BladeProcessor&, juce::AudioProcessorValueTreeState&);
    ~RebornEditor() override;

    void paint   (juce::Graphics&) override;
    void resized () override;

private:
    // ---- Timer (60fps VU refresh) ----
    void timerCallback() override;

    // ---- Layout constants ----
    static constexpr int W = 620;
    static constexpr int H = 380;

    // ---- Sub-components ----
    struct BladeVU : public juce::Component {
        float level { 0.f };
        bool  active { false };
        juce::String label;
        void paint(juce::Graphics& g) override;
    };

    std::array<BladeVU, 6> bladeVUs;

    // ---- Knobs + Labels ----
    juce::Slider threshKnob,  sensitivityKnob;
    juce::Label  threshLabel, sensitivityLabel;
    juce::ComboBox scaleBox, midiChBox;
    juce::Label    scaleLabel, midiChLabel;
    juce::ToggleButton polyToggle;
    juce::Label  titleLabel, versionLabel;

    // ---- APVTS attachments ----
    using SliderAttach = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttach  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttach = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttach> threshAttach, sensitivityAttach;
    std::unique_ptr<ComboAttach>  scaleAttach, midiChAttach;
    std::unique_ptr<ButtonAttach> polyAttach;

    // ---- Colors (design system) ----
    static constexpr juce::uint32 COL_BG       = 0xFF0D0D12;
    static constexpr juce::uint32 COL_PANEL    = 0xFF161622;
    static constexpr juce::uint32 COL_ACCENT   = 0xFF00FFB2;  // neon teal
    static constexpr juce::uint32 COL_ACCENT2  = 0xFFFF3CAC;  // neon magenta
    static constexpr juce::uint32 COL_BLADE    = 0xFF1E2235;
    static constexpr juce::uint32 COL_TEXT     = 0xFFE0E8FF;
    static constexpr juce::uint32 COL_SUBTEXT  = 0xFF6B7FAB;

    BladeProcessor& processor;
    juce::AudioProcessorValueTreeState& apvts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RebornEditor)
};

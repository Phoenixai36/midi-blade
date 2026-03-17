#include "RebornEditor.h"
#include "BladeProcessor.h"
#include "RebornParameters.h"

// ---- BladeVU paint ---------------------------------------------------
void RebornEditor::BladeVU::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.f);

    // Background
    g.setColour(juce::Colour(0xFF1E2235));
    g.fillRoundedRectangle(bounds, 4.f);

    // Level bar (bottom-up)
    if (level > 0.001f) {
        float barH = bounds.getHeight() * juce::jmin(level, 1.f);
        auto barBounds = bounds.removeFromBottom(barH);
        juce::Colour barCol = active
            ? juce::Colour(0xFF00FFB2)   // neon teal active
            : juce::Colour(0xFF1A5C45);  // dim inactive
        g.setColour(barCol);
        g.fillRoundedRectangle(barBounds, 3.f);
    }

    // Border glow when active
    if (active) {
        g.setColour(juce::Colour(0xFF00FFB2).withAlpha(0.6f));
        g.drawRoundedRectangle(bounds, 4.f, 1.5f);
    }

    // String label (E A D G B e)
    g.setColour(juce::Colour(0xFF6B7FAB));
    g.setFont(juce::Font(juce::FontOptions().withHeight(11.f)));
    g.drawText(label, getLocalBounds(), juce::Justification::centredBottom);
}

// ---- Constructor -----------------------------------------------------
RebornEditor::RebornEditor(BladeProcessor& p,
                           juce::AudioProcessorValueTreeState& state)
    : AudioProcessorEditor(&p), processor(p), apvts(state)
{
    setSize(W, H);
    setResizable(false, false);

    // ---- String labels ----
    const char* labels[] = { "E", "A", "D", "G", "B", "e" };
    for (int i = 0; i < 6; ++i) {
        bladeVUs[i].label = labels[i];
        addAndMakeVisible(bladeVUs[i]);
    }

    // ---- Title ----
    titleLabel.setText("REBORN", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(juce::FontOptions().withHeight(28.f).withStyle("Bold")));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(COL_ACCENT));
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    versionLabel.setText("GUITAR  v2.0", juce::dontSendNotification);
    versionLabel.setFont(juce::Font(juce::FontOptions().withHeight(11.f)));
    versionLabel.setColour(juce::Label::textColourId, juce::Colour(COL_SUBTEXT));
    versionLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(versionLabel);

    // ---- Threshold knob ----
    threshKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    threshKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 16);
    threshKnob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(COL_ACCENT));
    threshKnob.setColour(juce::Slider::thumbColourId,            juce::Colour(COL_ACCENT));
    threshKnob.setColour(juce::Slider::textBoxTextColourId,      juce::Colour(COL_TEXT));
    threshKnob.setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(COL_PANEL));
    threshLabel.setText("THRESHOLD", juce::dontSendNotification);
    threshLabel.setFont(juce::Font(juce::FontOptions().withHeight(10.f)));
    threshLabel.setColour(juce::Label::textColourId, juce::Colour(COL_SUBTEXT));
    threshLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(threshKnob); addAndMakeVisible(threshLabel);

    // ---- Sensitivity knob ----
    sensitivityKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    sensitivityKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 16);
    sensitivityKnob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(COL_ACCENT2));
    sensitivityKnob.setColour(juce::Slider::thumbColourId,             juce::Colour(COL_ACCENT2));
    sensitivityKnob.setColour(juce::Slider::textBoxTextColourId,       juce::Colour(COL_TEXT));
    sensitivityKnob.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(COL_PANEL));
    sensitivityLabel.setText("SENSITIVITY", juce::dontSendNotification);
    sensitivityLabel.setFont(juce::Font(juce::FontOptions().withHeight(10.f)));
    sensitivityLabel.setColour(juce::Label::textColourId, juce::Colour(COL_SUBTEXT));
    sensitivityLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(sensitivityKnob); addAndMakeVisible(sensitivityLabel);

    // ---- Scale ComboBox ----
    for (int i = 0; i < 8; ++i)
        scaleBox.addItem(RebornParams::SCALE_NAMES[i], i + 1);
    scaleBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(COL_PANEL));
    scaleBox.setColour(juce::ComboBox::textColourId,       juce::Colour(COL_TEXT));
    scaleBox.setColour(juce::ComboBox::arrowColourId,      juce::Colour(COL_ACCENT));
    scaleLabel.setText("SCALE", juce::dontSendNotification);
    scaleLabel.setFont(juce::Font(juce::FontOptions().withHeight(10.f)));
    scaleLabel.setColour(juce::Label::textColourId, juce::Colour(COL_SUBTEXT));
    addAndMakeVisible(scaleBox); addAndMakeVisible(scaleLabel);

    // ---- MIDI Channel ComboBox ----
    for (int i = 1; i <= 16; ++i)
        midiChBox.addItem("Ch " + juce::String(i), i);
    midiChBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(COL_PANEL));
    midiChBox.setColour(juce::ComboBox::textColourId,       juce::Colour(COL_TEXT));
    midiChBox.setColour(juce::ComboBox::arrowColourId,      juce::Colour(COL_ACCENT));
    midiChLabel.setText("MIDI OUT", juce::dontSendNotification);
    midiChLabel.setFont(juce::Font(juce::FontOptions().withHeight(10.f)));
    midiChLabel.setColour(juce::Label::textColourId, juce::Colour(COL_SUBTEXT));
    addAndMakeVisible(midiChBox); addAndMakeVisible(midiChLabel);

    // ---- Poly toggle ----
    polyToggle.setButtonText("POLY");
    polyToggle.setColour(juce::ToggleButton::textColourId,      juce::Colour(COL_TEXT));
    polyToggle.setColour(juce::ToggleButton::tickColourId,      juce::Colour(COL_ACCENT));
    polyToggle.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(COL_SUBTEXT));
    addAndMakeVisible(polyToggle);

    // ---- APVTS Attachments ----
    threshAttach      = std::make_unique<SliderAttach>(apvts, RebornParams::THRESHOLD,   threshKnob);
    sensitivityAttach = std::make_unique<SliderAttach>(apvts, RebornParams::SENSITIVITY, sensitivityKnob);
    scaleAttach       = std::make_unique<ComboAttach> (apvts, RebornParams::SCALE,       scaleBox);
    midiChAttach      = std::make_unique<ComboAttach> (apvts, RebornParams::MIDI_CH,     midiChBox);
    polyAttach        = std::make_unique<ButtonAttach>(apvts, RebornParams::POLY_MODE,   polyToggle);

    startTimerHz(60);
}

RebornEditor::~RebornEditor() { stopTimer(); }

// ---- Timer (VU meter update) -----------------------------------------
void RebornEditor::timerCallback()
{
    // Animate VU: decay each blade level
    for (auto& vu : bladeVUs) {
        vu.level  *= 0.85f;  // smooth decay
        vu.active  = (vu.level > 0.05f);
        vu.repaint();
    }
}

// ---- Paint -----------------------------------------------------------
void RebornEditor::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(juce::Colour(COL_BG));

    // Top header bar
    g.setColour(juce::Colour(COL_PANEL));
    g.fillRect(0, 0, W, 56);

    // Accent line under header
    g.setColour(juce::Colour(COL_ACCENT));
    g.fillRect(0, 55, W, 2);

    // Blade section background
    g.setColour(juce::Colour(COL_PANEL));
    g.fillRoundedRectangle(12.f, 70.f, 200.f, 240.f, 8.f);

    // Params section background
    g.setColour(juce::Colour(COL_PANEL));
    g.fillRoundedRectangle(224.f, 70.f, 384.f, 240.f, 8.f);

    // Section labels
    g.setFont(juce::Font(juce::FontOptions().withHeight(9.f)));
    g.setColour(juce::Colour(COL_SUBTEXT));
    g.drawText("HEX BLADES",  14,  73, 100, 12, juce::Justification::left);
    g.drawText("PARAMETERS",  226, 73, 100, 12, juce::Justification::left);

    // Footer
    g.setColour(juce::Colour(COL_SUBTEXT));
    g.setFont(juce::Font(juce::FontOptions().withHeight(9.f)));
    g.drawText("MCP API  :9000   MIDI 2.0 UMP", 12, H - 18, 300, 14,
               juce::Justification::left);
    g.drawText("phoenixai36 / midi-blade", W - 160, H - 18, 148, 14,
               juce::Justification::right);
}

// ---- Resized ---------------------------------------------------------
void RebornEditor::resized()
{
    // ---- Header ----
    titleLabel  .setBounds(16,  8, 160, 28);
    versionLabel.setBounds(16, 36,  160, 14);

    // ---- 6 Blade VU meters ----
    int vuX = 18, vuY = 88;
    int vuW = 26, vuH = 200;
    int vuGap = 5;
    for (int i = 0; i < 6; ++i)
        bladeVUs[i].setBounds(vuX + i * (vuW + vuGap), vuY, vuW, vuH);

    // ---- Knobs (params panel) ----
    int px = 240, py = 90;
    threshKnob    .setBounds(px,       py,  80, 90);
    threshLabel   .setBounds(px,       py + 88, 80, 14);
    sensitivityKnob.setBounds(px + 95, py,  80, 90);
    sensitivityLabel.setBounds(px + 95, py + 88, 80, 14);

    // ---- Combos ----
    scaleLabel .setBounds(px,       py + 112, 80,  12);
    scaleBox   .setBounds(px,       py + 124, 120, 26);
    midiChLabel.setBounds(px + 140, py + 112, 80,  12);
    midiChBox  .setBounds(px + 140, py + 124, 80,  26);

    // ---- Poly toggle ----
    polyToggle.setBounds(px, py + 166, 80, 24);
}

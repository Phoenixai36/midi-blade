cmake_minimum_required(VERSION 3.22)
project(MidiBlade VERSION 1.0.0)

# JUCE 7 path
add_subdirectory(JUCE)

juce_add_plugin(MidiBlade
    PLUGIN_MANUFACTURER_CODE  Mblr
    PLUGIN_CODE               Mbld
    FORMATS                   VST3 LV2 Standalone
    PRODUCT_NAME              "MIDI Blade"
    PLUGIN_MANUFACTURER       "Phoenixai36"
    IS_MIDI_EFFECT            FALSE
    IS_SYNTH                  FALSE
    NEEDS_MIDI_INPUT          FALSE
    NEEDS_MIDI_OUTPUT         TRUE
)

target_sources(MidiBlade
    PRIVATE
        src/BladeProcessor.cpp
        src/UMPConverter.cpp
)

target_compile_definitions(MidiBlade
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

target_link_libraries(MidiBlade
    PRIVATE
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
)

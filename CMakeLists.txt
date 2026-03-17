cmake_minimum_required(VERSION 3.22)
project(RebornGuitar VERSION 2.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(REBORN_USE_TORCH "Enable LibTorch crepe model" OFF)

if(REBORN_USE_TORCH)
    find_package(Torch REQUIRED)
    message(STATUS "[RebornGuitar] LibTorch ON")
endif()

add_subdirectory(JUCE)

juce_add_plugin(RebornGuitar
    PLUGIN_MANUFACTURER_CODE  Rbrn
    PLUGIN_CODE               Rbg2
    FORMATS                   VST3 Standalone
    PRODUCT_NAME              "RebornGuitar"
    PLUGIN_MANUFACTURER       "Phoenixai36"
    PLUGIN_DESCRIPTION        "Polyphonic guitar-to-MIDI with MCP + MIDI 2.0"
    IS_MIDI_EFFECT            FALSE
    IS_SYNTH                  FALSE
    NEEDS_MIDI_INPUT          FALSE
    NEEDS_MIDI_OUTPUT         TRUE
    VST3_CATEGORIES           "Instrument" "Fx"
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
)

target_sources(RebornGuitar PRIVATE
    src/BladeProcessor.cpp
    src/BladeProcessor.h
    src/BladeDSP.cpp
    src/BladeDSP.h
    src/BladeFilters.h
    src/UMPConverter.cpp
    src/UMPConverter.h
    src/RebornEditor.cpp
    src/RebornEditor.h
    src/RebornParameters.h
    src/MCPServer.cpp
    src/MCPServer.h
)

target_compile_definitions(RebornGuitar PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    $<$<BOOL:${REBORN_USE_TORCH}>:REBORN_USE_TORCH>
)

target_link_libraries(RebornGuitar
    PRIVATE
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_events
        $<$<BOOL:${REBORN_USE_TORCH}>:${TORCH_LIBRARIES}>
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

if(REBORN_USE_TORCH)
    target_include_directories(RebornGuitar PRIVATE ${TORCH_INCLUDE_DIRS})
endif()

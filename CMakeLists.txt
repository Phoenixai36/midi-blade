cmake_minimum_required(VERSION 3.22)
project(RebornGuitar VERSION 2.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# LibTorch (optional) – set REBORN_USE_TORCH=ON to enable crepe
# Usage: cmake -B build -DREBORN_USE_TORCH=ON -DTorch_DIR=/path/to/libtorch/share/cmake/Torch
# ============================================================
option(REBORN_USE_TORCH "Enable LibTorch crepe model (98% poly accuracy)" OFF)

if(REBORN_USE_TORCH)
    find_package(Torch REQUIRED)
    message(STATUS "[RebornGuitar] LibTorch found – crepe enabled")
else()
    message(STATUS "[RebornGuitar] LibTorch OFF – using IIR blade tracker")
endif()

# ============================================================
# JUCE 7  –  add as submodule: git submodule add https://github.com/juce-framework/JUCE
# ============================================================
add_subdirectory(JUCE)

juce_add_plugin(RebornGuitar
    PLUGIN_MANUFACTURER_CODE  Rbrn
    PLUGIN_CODE               Rbg1
    FORMATS                   VST3 Standalone          # FIX: dropped LV2 (needs valid URI) and VST2 (conflicts VST3)
    PRODUCT_NAME              "RebornGuitar"
    PLUGIN_MANUFACTURER       "Phoenixai36"
    PLUGIN_DESCRIPTION        "Polyphonic guitar-to-MIDI: IIR hex blades + optional crepe AI model"
    LV2URI                    "urn:phoenixai36:reborguitar"   # kept for future LV2, won't build unless added back to FORMATS
    IS_MIDI_EFFECT            FALSE
    IS_SYNTH                  FALSE
    NEEDS_MIDI_INPUT          FALSE
    NEEDS_MIDI_OUTPUT         TRUE
    VST3_CATEGORIES           "Instrument" "Fx"
)

target_sources(RebornGuitar
    PRIVATE
        src/BladeDSP.cpp
        src/BladeDSP.h
        src/BladeProcessor.cpp
        src/BladeProcessor.h
        src/BladeFilters.h
        src/UMPConverter.cpp
        src/UMPConverter.h
)

target_compile_definitions(RebornGuitar
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0                  # FIX: disables VST2/VST3 automation conflict check
        $<$<BOOL:${REBORN_USE_TORCH}>:REBORN_USE_TORCH>
)

target_link_libraries(RebornGuitar
    PRIVATE
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_dsp
        $<$<BOOL:${REBORN_USE_TORCH}>:"${TORCH_LIBRARIES}">
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

if(REBORN_USE_TORCH)
    target_include_directories(RebornGuitar PRIVATE ${TORCH_INCLUDE_DIRS})
endif()

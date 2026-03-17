#pragma once
// Central parameter IDs — shared between processor, editor, MCP
namespace RebornParams {
    static constexpr auto THRESHOLD   = "threshold";
    static constexpr auto SENSITIVITY = "sensitivity";
    static constexpr auto MIDI_CH     = "midi_ch";
    static constexpr auto POLY_MODE   = "poly_mode";
    static constexpr auto ROOT_NOTE   = "root_note";
    static constexpr auto SCALE       = "scale";

    // Scale names for UI
    static const char* SCALE_NAMES[] = {
        "Chromatic", "Major", "Minor", "Pentatonic",
        "Blues", "Dorian", "Phrygian", "Mixolydian"
    };

    // Scale intervals (semitones from root)
    static const int SCALE_INTERVALS[8][12] = {
        {0,1,2,3,4,5,6,7,8,9,10,11},   // Chromatic
        {0,2,4,5,7,9,11,-1},            // Major
        {0,2,3,5,7,8,10,-1},            // Minor
        {0,2,4,7,9,-1},                 // Pentatonic
        {0,3,5,6,7,10,-1},              // Blues
        {0,2,3,5,7,9,10,-1},            // Dorian
        {0,1,3,5,7,8,10,-1},            // Phrygian
        {0,2,4,5,7,9,10,-1},            // Mixolydian
    };
}

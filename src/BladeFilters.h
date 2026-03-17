#pragma once

// Blade frequency map – Standard tuning (EADGBE)
namespace BladeFreqs {
    constexpr float E2 = 82.41f;
    constexpr float A2 = 110.00f;
    constexpr float D3 = 146.83f;
    constexpr float G3 = 196.00f;
    constexpr float B3 = 246.94f;
    constexpr float E4 = 329.63f;

    constexpr float ALL[6] = { E2, A2, D3, G3, B3, E4 };
    constexpr const char* NAMES[6] = { "E2", "A2", "D3", "G3", "B3", "E4" };
}

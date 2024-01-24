#pragma once

#include "../Control.hpp"
#include "Arduino.h"
#include "../audio_externs.h"


namespace audio {

class VASynth {
public:
    Control<float> amplitude {"Amp.", 1, {0.01, 1}, [](float a) { sine1.amplitude(a); }};
    Control<float> frequency {"Freq.", 440, {1, 20000}, [](float f) { sine1.frequency(f); }};

    /// @brief The cutoff frequency of the filter.
    Control<float> filterCutoffFreq {"Cutoff", 8000};

    /// @brief Update synth operation from current controls.
    void updateFromControls();
};


extern VASynth va_module;

}
#pragma once

#include "../Control.hpp"
#include "Arduino.h"
#include "../audio_externs.h"


namespace audio {

class VASynth {
public:
    Control<float> amplitude {"Amp.", 1, {0.01, 1}, [](float a) { sine1.amplitude(a); }};
    Control<float> frequency {"Freq.", 440, {1, 20000}, [](float f) { sine1.frequency(f); va_osc1.frequency(f); va_osc2.frequency(f); }};
    
    /// @brief Mix between osc1 and osc2.
    Control<float> osc12Mix {"Mix.1&2", 0, {0, 1}, [](float mix) { 
        va_osc_mixer.gain(0, 1.f - mix);
        va_osc_mixer.gain(1, mix);
    }};

    /// @brief The cutoff frequency of the filter.
    Control<float> filterCutoffFreq {"Cutoff", 4000, {20, 20000}, [](float freq) { va_filter.frequency(freq); } };

    /// @brief Resonance of the filter.
    Control<float> filterResonance {"Resonance", 0.7f, {0.7f, 7.f}, [](float q) { va_filter.resonance(q); } };

    /// @brief Select the type of filter. 0, 1, 2 for low-pass, band-pass, and high-pass respectively.
    Control<int> filterSwitch {"FltType", 0, {0, 2}, [](int choice) {
        float lg = 1, bg = 0, hg = 0;
        switch (choice) {
        // case 0: // default case handled by init above
        //     lg = 1; bg = 0; hg = 0;
        //     break;
        case 1:
            lg = 0; bg = 1; hg = 0;
            break;
        case 2:
            lg = 0; bg = 0; hg = 1;
            break;
        }

        va_filter_mixer.gain(0, lg);
        va_filter_mixer.gain(1, bg);
        va_filter_mixer.gain(2, hg);
    }};

    Control<int> dummyContrl {"Dummy", 0};

    Control<int> osc1Type {"Type.Osc1", 0, {0, 12}, [](int choice) { va_osc1.begin(choice); }};
    Control<int> osc2Type {"Type.Osc2", 0, {0, 12}, [](int choice) { va_osc2.begin(choice); }};

    /// @brief Update synth operation from current controls.
    void updateFromControls();
};


extern VASynth va_module;

}
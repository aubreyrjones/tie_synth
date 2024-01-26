#pragma once

#include "../Control.hpp"
#include "Arduino.h"
#include "../audio_externs.h"


namespace audio {

class VASynth {
public:
    // Oscillators 1 and 2 are simple waveforms.
    struct Osc {
        AudioSynthWaveform &osc;
        Osc(AudioSynthWaveform &o) : osc(o) {};

        /// @brief Wavetype of oscillator.
        Control<int> waveType {"Type", 0, {0, 12}, [this](int choice) { osc.begin(choice); }};

        /// @brief Phase of oscillator.
        Control<float> phase {"Phase", 0, {0, 360}, [this](float p) { osc.phase(p); }};

        /// @brief Pulse width if pulse wave type chosen.
        Control<float> pulseWidth {"P.Width", 0.5, {0, 1}, [this](float pw) { osc.pulseWidth(pw); }};

        /// @brief How many cents to detune the pitch.
        Control<int> detune {"Detune", 0, {-10, 10}};

        /// @brief Set the amplitude of the wave.
        Control<float> amplitude {"Amp.", 0.8, {0.01, 1}, [this](float a) { osc.amplitude(a); }};

    } osc1{va_osc1}, osc2{va_osc2};


    // Oscillator 3 is capable of basic FM, feeding from input or Osc 2.
    struct FMOsc {
        AudioSynthWaveformModulated &osc;
        FMOsc(AudioSynthWaveformModulated &o) : osc(o) {};

        /// @brief Wavetype of oscillator.
        Control<int> waveType {"Type", 0, {0, 12}, [this](int choice) { osc.begin(choice); }};

        /// @brief How many cents to detune the pitch.
        Control<int> detune {"Detune", 0, {-10, 10}};

        /// @brief Set the amplitude of the wave.

        Control<float> amplitude {"Amp.", 0.8, {0.01, 1}, [this](float a) { osc.amplitude(a); }};

        Control<float> osc1Level {"FM.Osc1", 0, {0, 1}, [this](float l) { va_fm_mod_mixer.gain(0, l); }};
        Control<float> osc2Level {"FM.Osc2", 0, {0, 1}, [this](float l) { va_fm_mod_mixer.gain(1, l); }};
        Control<float> inLLevel {"FM.InL", 0, {0, 1}, [this](float l) { va_fm_mod_mixer.gain(2, l); }};
        Control<float> inRLevel {"FM.InR", 0, {0, 1}, [this](float l) { va_fm_mod_mixer.gain(3, l); }};


    } osc3{va_osc3};

    Control<int> osc1Type {"Type.Osc1", 0, {0, 12}, [](int choice) { va_osc1.begin(choice); }};
    Control<int> osc2Type {"Type.Osc2", 0, {0, 12}, [](int choice) { va_osc2.begin(choice); }};

    /// @brief Mix between osc1 and osc2.
    Control<float> osc12Mix {"Mix.1&2", 0, {0, 1}, [](float mix) { 
        va_osc_mixer.gain(0, 1.f - mix);
        va_osc_mixer.gain(1, mix);
    }};

    struct OscMix {
        Control<float> osc1 {"Mix.Osc1", 1, {0, 1}, [this](float l) { va_osc_mixer.gain(0, l); }};
        Control<float> osc2 {"Mix.Osc2", 0, {0, 1}, [this](float l) { va_osc_mixer.gain(1, l); }};
        Control<float> osc3 {"Mix.Osc3", 0, {0, 1}, [this](float l) { va_osc_mixer.gain(2, l); }};
    } mix;


    /// @brief The cutoff frequency of the filter.
    Control<float> filterCutoffFreq {"Cutoff", 4000, {20, 20000}, [](float freq) { va_filter.frequency(freq); } };

    /// @brief Resonance of the filter.
    Control<float> filterResonance {"Resonance", 0.7f, {0.7f, 7.f}, [](float q) { va_filter.resonance(q); } };

    /// @brief Select the type of filter. 0, 1, 2 for low-pass, band-pass, and high-pass respectively.
    Control<int> filterSwitch {"Type.Fltr", 0, {0, 2}, [](int choice) {
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

    Control<float> wavefolderAmount {"Wavefold", 0.05f, {0.05f, 1.f}, [](float a) { va_wavefolder_control.amplitude(a); }};

    Control<int> dummyContrl {"Dummy", 0};

    Control<float> amplitude {"Amp.", 0.8, {0.01, 1}, [](float a) { va_osc1.amplitude(a); va_osc2.amplitude(a); }};

    /// Set the overall frequency for the VA module.
    Control<float> frequency {"Freq.", 440, {1, 20000}, [](float f) { 
        va_osc1.frequency(f); va_osc2.frequency(f); 
    }};

    /// @brief Set up things not handled by Controls.
    void doSetup();
};


extern VASynth va_module;

}
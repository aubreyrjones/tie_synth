#pragma once

#include "../Control.hpp"
#include "Arduino.h"
#include "../audio_externs.h"
#include "Analyzer.hpp"
#include "../Synth.hpp"

namespace audio {

struct AdditiveSynth : public Synth {
    Control<float> spectralMix {"Mix.Spect", 0, {0, 2}, [this](float g) { add_mixer.gain(0, g); }};
    Control<float> banksMix {"Mix.Banks", 1, {0, 2}, [this](float g) { add_mixer.gain(1, g); }};

    Control<float> frequency {"Freq.", 172, {1, 20000}, [this](float f) {
        oscbank1.frequency(0, f);
        oscbank1.setActive(0, true);
    }};

    Control<int> debug {"Debug", 0, {0, 1}, [](int b) { oscbank1.debug(b); }};

    AudioAnalyzer analyzer;

    decltype(additive1.partials()) partials() { return additive1.partials(); }

    decltype(oscbank1.getVoice()) bankVoice() { return oscbank1.getVoice(); }

    void doSetup();

    virtual void noteOn(NoteNumber note, float velocity) override;
    virtual void noteOff(NoteNumber note, float velocity) override;
    virtual void controlChange(CCNumber cc, byte value) override;
};

extern AdditiveSynth as_module;

}
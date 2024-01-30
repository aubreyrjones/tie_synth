#pragma once

#include "../Control.hpp"
#include "Arduino.h"
#include "../audio_externs.h"


namespace audio {

struct AdditiveSynth {
    Control<float> frequency {"Freq.", 172, {1, 20000}, [this](float f) {
        additive1.frequency(f);
    }};

    Control<int> debug {"Debug", 0, {0, 1}, [](int b) { additive1.window(b); }};

    decltype(additive1.partials()) partials() { return additive1.partials(); }
};

extern AdditiveSynth as_module;

}
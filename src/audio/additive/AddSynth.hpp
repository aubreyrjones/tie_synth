#pragma once

#include "../Control.hpp"
#include "Arduino.h"
#include "../audio_externs.h"


namespace audio {

struct AdditiveSynth {
    Control<float> frequency {"Freq.", 689, {1, 20000}, [this](float f) {
        additive1.frequency(f);
    }};

    decltype(additive1.partials()) partials() { return additive1.partials(); }
};

extern AdditiveSynth as_module;

}